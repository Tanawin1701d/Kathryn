# Karray element-record declaration — `kaf()` field descriptors, the reusable
# `KBundle` record (Chisel-Bundle style, bundles nest), and the shared
# field-collection walk both `KBundle` and `Karray` subclasses use.
#
# - The Rust core only ever sees a FLAT `(name, width)` list: a nested bundle
#   flattens at declaration with an underscore prefix (`pos = kaf(Vec2)` ->
#   leaf "pos_x"); attribute access rebuilds the same flat name (`d[0].pos.x`).
#   Duplicate flat names are rejected here.
# - THE RECORD IS FINISHED AT INSTANTIATION — the keyword's VALUE picks:
#       Entry(REG, (4,), "e", data=16,          # int   -> width of a DECLARED field
#                             spectag=kaf(8))   # kaf() -> a field only THIS array has
#   A class-body `kaf(w)` width is a DEFAULT; `kaf()` with no width must be
#   sized by every instantiation (it rides the flat list as `(name, None)`);
#   a call-site kaf() APPENDS after the declared fields, in keyword order,
#   flattened through the same walk a class-body bundle takes.
# - `resolve_karray_field_specs` is the ONE place a final width is decided and
#   the ONE place a field is added; the class's own list is never mutated, so
#   two arrays of one class may differ.

from __future__ import annotations

from typing import Any, Callable, Dict, Final, Iterable, List, Optional, Sequence, Set, Tuple, Union


class KarrayField:
    """Field descriptor for Karray/KBundle subclasses — one kaf() spec.

        class Entry(Karray):
            valid = kaf(1)
            pos   = kaf(Vec2)   # nested bundle -> leaf fields pos_x, pos_y
            data  = kaf()       # width required at instantiation
            tag   = kaf(8)      # 8 unless the instantiation says otherwise
    """

    __slots__ = ("width", "subtype", "name")
    width   : Optional[int]     # leaf width; None = bundle OR pending width
    subtype : Optional[type]    # bundle class (declares __karray_fields__), else None
    name    : Optional[str]     # field name; filled by __set_name__ in a class body

    def __init__(self, width_or_type: Union[int, type, None] = None, name: Optional[str] = None) -> None:
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

    def __set_name__(self, owner: type, attr_name: str) -> None:
        if self.name is None:
            self.name = attr_name


def kaf(width_or_type: Union[int, type, None] = None, name: Optional[str] = None) -> KarrayField:
    """Declare one element field.

    - `kaf(width)`      — a leaf; the width is a DEFAULT an instantiation may
                          override (`Entry(REG, (4,), "e", data=16)`).
    - `kaf(BundleType)` — nests that bundle's fields under this name ('_'-flat).
    - `kaf()`           — a leaf with NO default: every instantiation must size
                          it ("this number is the caller's to choose").
    - In a class body the attribute name becomes the field name; `name`
      overrides it (the only route to a non-identifier field name).
    - Passed at an instantiation, the same spec ADDS a field to that one array
      (`Entry(REG, (4,), "e", spectag=kaf(8))`).
    """
    return KarrayField(width_or_type, name)


# Karray.__init__ takes field arguments as keywords, so a field may not be named
# after one of its own parameters — `Entry(REG, (4,), name=8)` cannot mean two
# things. Declaring one raises when the class is created, not here.
RESERVED_FIELD_NAMES : Final[Tuple[str, ...]] = ("backing", "shape", "name")


def _expand_field_spec(name: str, spec: KarrayField, add: Callable[[str, Optional[int]], None]) -> None:
    """Emit the flat leaves one kaf() spec stands for, under `name`.

    - Leaf: itself. Bundle: splices its already-flat fields under `name + '_'`
      (pos = kaf(Vec2) -> "pos_x", "pos_y"); deeper nesting flattened when the
      bundle class was defined, so ONE prefix level is ever needed.
    - Shared by the class-body walk AND the instantiation path, so an added
      field flattens exactly like a declared one.
    """
    if spec.subtype is None:
        add(name, spec.width)
    else:
        for sub_name, sub_width in spec.subtype.__karray_fields__:
            add(f"{name}_{sub_name}", sub_width)


def _final_width(width: Optional[int], name: str, where: str, added: bool) -> int:
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


def resolve_karray_field_specs(
    fields    : Sequence[Tuple[str, Optional[int]]],
    overrides : Dict[str, Union[int, KarrayField]],
    where     : str,
) -> List[Tuple[str, int]]:
    """Final (name, width) list for ONE instantiation — the one place a width
    is decided and the one place a field is added.

    - `fields`: the class's flat declaration list (width None = kaf() with no
      default).
    - `overrides` keyword VALUE picks: int -> width for a DECLARED field;
      kaf() spec -> a field this array has and the class does not.
    - Added fields append after the declared ones, in keyword order.
    """
    declared_names = [name for name, _ in fields]
    declared       = set(declared_names)

    widths : Dict[str, int]                = {}   # declared field -> width
    added  : List[Tuple[str, KarrayField]] = []   # (field name, spec), keyword order
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

    resolved : List[Tuple[str, int]] = []
    seen     : Set[str]              = set()

    def emit(name: str, width: Optional[int], is_added: bool) -> None:
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


def get_declared_karray_fields(cls: type) -> Tuple[Tuple[str, Optional[int]], ...]:
    """Return the normalized FLAT field declarations collected on a subclass.

    A width of None means kaf() declared no default — see resolve_karray_field_specs.
    """
    return tuple(getattr(cls, "__karray_fields__", ()))


def collect_declared_karray_fields(cls: type) -> Tuple[Tuple[str, Optional[int]], ...]:
    """The shared __init_subclass__ walk: inherited flat fields first (MRO
    base→parent order), then cls's own kaf() specs — a bundle spec expands to
    its (already flat) fields prefixed with the spec name + '_'."""
    fields : List[Tuple[str, Optional[int]]] = []
    seen   : Set[str]                        = set()

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

    # Flat (name, width) leaves; width None = kaf() with no default.
    __karray_fields__ : Tuple[Tuple[str, Optional[int]], ...] = ()

    def __init_subclass__(cls, **kwargs: Any) -> None:
        super().__init_subclass__(**kwargs)
        cls.__karray_fields__ = collect_declared_karray_fields(cls)


def normalize_karray_field_specs(fields: Iterable[Tuple[str, int]]) -> List[Tuple[str, int]]:
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
