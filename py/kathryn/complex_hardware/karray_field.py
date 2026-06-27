from __future__ import annotations

from typing import Optional, Tuple


class KarrayField:
    """Field descriptor used by Karray subclasses.

    Example:
        class Src(Karray):
            valid = kaf(1)
            data  = kaf(8)
    """

    __slots__ = ("width", "name")

    def __init__(self, width: int, name: Optional[str] = None) -> None:
        self.width = int(width)
        self.name  = None if name is None else str(name)

    def __set_name__(self, owner, attr_name: str) -> None:
        if self.name is None:
            self.name = attr_name


def kaf(width: int, name: Optional[str] = None) -> KarrayField:
    """Create a Karray field descriptor for subclass-based element layouts."""
    return KarrayField(width, name)


def get_declared_karray_fields(cls) -> Tuple[Tuple[str, int], ...]:
    """Return the normalized field declarations collected on a Karray subclass."""
    return tuple(getattr(cls, "__karray_fields__", ()))


def normalize_karray_field_specs(fields) -> list:
    """Convert (name, width) pair iterables into a list of (str, int) tuples."""
    try:
        return [(str(n), int(w)) for (n, w) in fields]
    except TypeError as exc:
        raise TypeError(
            "Karray fields must be (name, width) pairs or declared with kaf() "
            "on a Karray subclass"
        ) from exc
