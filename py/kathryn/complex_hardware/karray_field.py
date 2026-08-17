# Karray element-record declaration: `kaf()` field descriptors, the reusable
# `KBundle` record type (Chisel-Bundle style — bundles nest inside bundles), and
# the shared field-collection walk both `KBundle` and `Karray` subclasses use.
#
# The Rust core only ever sees a FLAT `(name, width)` list: a nested bundle
# field flattens at declaration time with an underscore-joined prefix
# (`pos = kaf(Vec2)` with `Vec2.x` -> leaf field "pos_x"), and attribute access
# rebuilds the same flat name (`d[0].pos.x` -> field "pos_x"). Duplicate flat
# names (e.g. a literal leaf "pos_x" next to bundle pos{x}) are rejected here.
#
# THE RECORD IS FINISHED AT INSTANTIATION. A class body states the shape a
# record USUALLY has; the call that builds one array settles it. Two things move
# to the call, and the keyword's VALUE is what picks between them:
#
#     Entry(REG, (4,), "e", data=16,          # an int  -> width for a DECLARED field
#                           spectag=kaf(8))   # a kaf() -> a field only THIS array has
#
# A `kaf(w)` width in a class body is therefore a default, `kaf()` with no width
# declares a field every instantiation must size, and a kaf() at the call adds
# one the class never mentioned (appended after the declared fields, in keyword
# order, flattened through the same walk a class-body bundle takes).
#
# The reason is that a class body cannot see a caller's parameters —
# `__init_subclass__` stamps `__karray_fields__` when the class is created — so
# without this, one record shape at two widths meant two classes built by hand
# with `type()`, and a record that grows one field for one pipeline meant a
# third. A generator sizing its arrays from a description (a CPU's PC width, an
# ISA's instruction length, a speculation tag only the out-of-order build has)
# would write a class factory for every record it owns. The declaration stays
# the readable class body; the variable part moves to the call.
#
# A pending width rides through the flat list as `(name, None)`.
# `resolve_karray_field_specs` below is the ONE place a final width is decided
# and the ONE place a field is added; the class's own field list is never
# mutated, so two arrays of one class can differ and the class still reads as
# what every array has in common.

from __future__ import annotations

from typing import Optional, Tuple


class KarrayField:
    """Field descriptor used by Karray/KBundle subclasses. Holds a leaf bit
    width, a nested bundle type (any class declaring __karray_fields__), or
    nothing at all — a leaf whose width arrives at instantiation.

    Example:
        class Vec2(KBundle):
            x = kaf(8)
            y = kaf(8)

        class Entry(Karray):
            valid = kaf(1)
            pos   = kaf(Vec2)          # nested bundle -> leaf fields pos_x, pos_y
            data  = kaf()              # width required at instantiation
            tag   = kaf(8)             # 8 unless the instantiation says otherwise
    """

    __slots__ = ("width", "subtype", "name")

    def __init__(self, width_or_type=None, name: Optional[str] = None) -> None:
        if width_or_type is None:
            # kaf(): a leaf with no declared width. It reaches the flat list as
            # (name, None) and Karray.__init__ demands a width for it.
            self.width   = None
            self.subtype = None
        elif isinstance(width_or_type, bool):
            raise TypeError("kaf() takes a bit width or a bundle type, not a bool")
        elif isinstance(width_or_type, int):
            self.width   = int(width_or_type)
            self.subtype = None
        elif hasattr(width_or_type, "__karray_fields__"):
            if not width_or_type.__karray_fields__:
                raise TypeError(f"kaf({width_or_type.__name__}): the bundle declares no fields")
            self.width   = None
            self.subtype = width_or_type
        else:
            raise TypeError("kaf() takes a bit width (int) or a KBundle/Karray subclass")
        self.name = None if name is None else str(name)

    def __set_name__(self, owner, attr_name: str) -> None:
        if self.name is None:
            self.name = attr_name


def kaf(width_or_type=None, name: Optional[str] = None) -> KarrayField:
    """Declare one element field.

    `kaf(width)`      — a leaf; the width is the DEFAULT and an instantiation
                        may override it (`Entry(REG, (4,), "e", data=16)`).
    `kaf(BundleType)` — nests that bundle's fields under this field's name,
                        flattened with '_'.
    `kaf()`           — a leaf with NO default: every instantiation must give
                        it a width, which is how a record says "this number is
                        the caller's to choose".

    In a class body the attribute name becomes the field name; `name` overrides
    it, which is the only way to reach a field name that is not a Python
    identifier. The same spec passed to an instantiation ADDS a field to that
    one array (`Entry(REG, (4,), "e", spectag=kaf(8))`).
    """
    return KarrayField(width_or_type, name)


# Karray.__init__ takes field arguments as keywords, so a field may not be named
# after one of its own parameters — `Entry(REG, (4,), name=8)` cannot mean two
# things. Declaring one raises when the class is created, not here.
RESERVED_FIELD_NAMES = ("backing", "shape", "name")


def _expand_field_spec(name: str, spec: KarrayField, add) -> None:
    """Emit the flat leaves one kaf() spec stands for, under `name`.

    A leaf is itself; a bundle splices in the bundle's already-flat fields with
    `name + '_'` (pos = kaf(Vec2) -> "pos_x", "pos_y"). Deeper nesting was
    flattened when the bundle class was defined, so ONE prefix level is all that
    is ever needed. Shared by the class-body walk and the instantiation path, so
    a field added at a call flattens exactly like a declared one.
    """
    if spec.subtype is None:
        add(name, spec.width)
    else:
        for sub_name, sub_width in spec.subtype.__karray_fields__:
            add(f"{name}_{sub_name}", sub_width)


def _final_width(width, name: str, where: str, added: bool) -> int:
    """Validate one resolved width. `added` only changes the advice on None."""
    if width is None:
        if added:
            raise TypeError(
                f"{where}: field '{name}' was added with kaf() and no width — an "
                f"added field must state one (e.g. {name}=kaf(8))")
        raise TypeError(
            f"{where}: field '{name}' was declared with kaf() and no width, "
            f"so the instantiation must give it one (e.g. {name}=32)")
    if isinstance(width, bool) or not isinstance(width, int):
        raise TypeError(
            f"{where}: width for field '{name}' must be an int, "
            f"got {type(width).__name__}")
    if width < 1:
        raise ValueError(
            f"{where}: width for field '{name}' must be >= 1, got {width}")
    return int(width)


def resolve_karray_field_specs(fields, overrides, where: str) -> list:
    """Final (name, width) list for ONE instantiation. The one place a width is
    decided, and the one place a field is added.

    `fields` is the class's flat declaration list, whose widths may be None
    (kaf() with no width). `overrides` maps a keyword to either:

        an int        — the width for a field the class DECLARES
        a kaf() spec  — a field this array has and the class does not

    The value's type is what picks: a number changes a field, a kaf() declares
    one. Added fields are appended after the declared ones, in the order the
    keywords were written.
    """
    declared_names = [name for name, _ in fields]
    declared       = set(declared_names)

    widths : dict = {}      # declared field -> width
    added  : list = []      # (field name, spec), in keyword order
    for key, value in overrides.items():
        if isinstance(value, KarrayField):
            if key in declared:
                raise TypeError(
                    f"{where}: field '{key}' is already declared on the class, so "
                    f"kaf() here would declare it twice — pass a width instead "
                    f"({key}=<bits>) to change the one that is there")
            # An explicit kaf(w, "other") name wins over the keyword, exactly as
            # it wins over the attribute name in a class body — it is the only
            # route to a field name that is not a Python identifier.
            added.append((spec_name if (spec_name := value.name) else key, value))
        else:
            if key not in declared:
                raise TypeError(
                    f"{where}: no field named '{key}' "
                    f"(declared: {', '.join(declared_names) or 'none'}) — write "
                    f"{key}=kaf(<bits>) to add it to this array")
            widths[key] = value

    resolved : list = []
    seen     : set  = set()

    def emit(name: str, width, is_added: bool) -> None:
        if name in seen:
            raise TypeError(
                f"{where}: duplicate field name '{name}' "
                f"(an added field collides with one already there)")
        resolved.append((str(name), _final_width(width, name, where, is_added)))
        seen.add(name)

    for name, declared_width in fields:
        emit(name, widths.get(name, declared_width), False)
    for name, spec in added:
        _expand_field_spec(name, spec, lambda n, w: emit(n, w, True))
    return resolved


def get_declared_karray_fields(cls) -> Tuple[Tuple[str, Optional[int]], ...]:
    """Return the normalized FLAT field declarations collected on a subclass.

    A width of None means kaf() declared no default — see resolve_karray_field_specs.
    """
    return tuple(getattr(cls, "__karray_fields__", ()))


def collect_declared_karray_fields(cls) -> Tuple[Tuple[str, Optional[int]], ...]:
    """The shared __init_subclass__ walk: inherited flat fields first (MRO
    base→parent order), then cls's own kaf() specs — a bundle spec expands to
    its (already flat) fields prefixed with the spec name + '_'."""
    fields: list = []
    seen  : set  = set()

    def add(name: str, width: Optional[int]) -> None:
        if name in seen:
            raise TypeError(f"duplicate Karray field name: {name}")
        fields.append((name, width))
        seen.add(name)

    # ---- inherited fields (oldest ancestor first) ----

    # __mro__[1:] is every ancestor of cls (cls itself excluded); reversed()
    # walks them most-basic-first so the oldest ancestor's fields keep the
    # lowest positions. Each base's __karray_fields__ is already FLAT (stamped
    # when that base class was defined), so nothing re-expands here. The same
    # field can surface through several bases' stamped lists (each base folds
    # in ITS ancestors), so a duplicate name here is the same field seen twice
    # — skip silently, don't raise.
    for base in reversed(cls.__mro__[1:]):
        for name, width in get_declared_karray_fields(base):
            if name not in seen:
                fields.append((name, width))
                seen.add(name)

    # ---- this class's own kaf() specs (definition order) ----

    # cls.__dict__ holds only THIS class's namespace — never inherited attrs —
    # in declaration order; anything that is not a kaf() descriptor is skipped.
    # Unlike the inherited pass, collisions here go through add() and RAISE:
    # a subclass may not re-declare a field name it already inherited.
    for _, spec in cls.__dict__.items():
        if not isinstance(spec, KarrayField):
            continue
        _expand_field_spec(spec.name, spec, add)

    return tuple(fields)   # immutable snapshot — becomes cls.__karray_fields__


class KBundle:
    """Reusable element record — declare kaf() fields on a subclass and nest it
    in a Karray (or another bundle) via `kaf(TheBundle)`. Pure type: never
    instantiated, it only carries the flattened field list."""

    __karray_fields__ = ()

    def __init_subclass__(cls, **kwargs) -> None:
        super().__init_subclass__(**kwargs)
        cls.__karray_fields__ = collect_declared_karray_fields(cls)


def normalize_karray_field_specs(fields) -> list:
    """Convert (name, width) pair iterables into a list of (str, int) tuples.

    Kept for a caller holding a raw pair list. Karray.__init__ goes through
    `resolve_karray_field_specs` instead, because a declared width may be None
    (kaf() with no default) and only the instantiation can settle it.
    """
    try:
        return [(str(n), int(w)) for (n, w) in fields]
    except TypeError as exc:
        raise TypeError(
            "Karray fields must be (name, width) pairs or declared with kaf() "
            "on a Karray subclass"
        ) from exc
