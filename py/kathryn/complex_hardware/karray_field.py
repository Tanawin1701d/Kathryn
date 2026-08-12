# Karray element-record declaration: `kaf()` field descriptors, the reusable
# `KBundle` record type (Chisel-Bundle style — bundles nest inside bundles), and
# the shared field-collection walk both `KBundle` and `Karray` subclasses use.
#
# The Rust core only ever sees a FLAT `(name, width)` list: a nested bundle
# field flattens at declaration time with an underscore-joined prefix
# (`pos = kaf(Vec2)` with `Vec2.x` -> leaf field "pos_x"), and attribute access
# rebuilds the same flat name (`d[0].pos.x` -> field "pos_x"). Duplicate flat
# names (e.g. a literal leaf "pos_x" next to bundle pos{x}) are rejected here.

from __future__ import annotations

from typing import Optional, Tuple


class KarrayField:
    """Field descriptor used by Karray/KBundle subclasses. Holds either a leaf
    bit width or a nested bundle type (any class declaring __karray_fields__).

    Example:
        class Vec2(KBundle):
            x = kaf(8)
            y = kaf(8)

        class Entry(Karray):
            valid = kaf(1)
            pos   = kaf(Vec2)          # nested bundle -> leaf fields pos_x, pos_y
    """

    __slots__ = ("width", "subtype", "name")

    def __init__(self, width_or_type, name: Optional[str] = None) -> None:
        if isinstance(width_or_type, bool):
            raise TypeError("kaf() takes a bit width or a bundle type, not a bool")
        if isinstance(width_or_type, int):
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


def kaf(width_or_type, name: Optional[str] = None) -> KarrayField:
    """Declare one element field: `kaf(width)` is a leaf, `kaf(BundleType)` nests
    that bundle's fields under this field's name (flattened with '_')."""
    return KarrayField(width_or_type, name)


def get_declared_karray_fields(cls) -> Tuple[Tuple[str, int], ...]:
    """Return the normalized FLAT field declarations collected on a subclass."""
    return tuple(getattr(cls, "__karray_fields__", ()))


def collect_declared_karray_fields(cls) -> Tuple[Tuple[str, int], ...]:
    """The shared __init_subclass__ walk: inherited flat fields first (MRO
    base→parent order), then cls's own kaf() specs — a bundle spec expands to
    its (already flat) fields prefixed with the spec name + '_'."""
    fields: list = []
    seen  : set  = set()

    def add(name: str, width: int) -> None:
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
        if spec.subtype is None:
            add(spec.name, spec.width)      # leaf: one (name, width) as declared
        else:
            # Bundle spec: splice in the bundle's already-flat fields under this
            # spec's name + '_' (pos = kaf(Vec2) -> "pos_x", "pos_y"). Deeper
            # nesting was flattened when the bundle class itself was defined,
            # so ONE prefix level per class is all that is ever needed.
            for sub_name, sub_width in spec.subtype.__karray_fields__:
                add(f"{spec.name}_{sub_name}", sub_width)

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
    """Convert (name, width) pair iterables into a list of (str, int) tuples."""
    try:
        return [(str(n), int(w)) for (n, w) in fields]
    except TypeError as exc:
        raise TypeError(
            "Karray fields must be (name, width) pairs or declared with kaf() "
            "on a Karray subclass"
        ) from exc
