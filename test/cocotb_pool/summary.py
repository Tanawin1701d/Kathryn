# Per-case result record + the ASCII summary table printed at the end of a run.

from __future__ import annotations

import sys
from dataclasses import dataclass


@dataclass
class CaseResult:
    case        : str    # "<tc>.<testcase>" identifier
    description : str
    status      : str       # PASS / FAIL / COMPILE / NO_RESULT / SKIP
    detail      : str = ""  # error message for COMPILE / NO_RESULT rows


# Status → (symbol, ANSI colour) for the summary table.
_STATUS_STYLE = {
    "PASS"     : ("PASS",        "\033[32m"),  # green
    "FAIL"     : ("FAIL",        "\033[31m"),  # red
    "COMPILE"  : ("COMPILE-ERR", "\033[35m"),  # magenta
    "NO_RESULT": ("NO-RESULT",   "\033[33m"),  # yellow
    "SKIP"     : ("SKIP",        "\033[90m"),  # grey
}
_RESET = "\033[0m"


def print_summary(results: list[CaseResult]) -> None:
    use_color = sys.stdout.isatty()

    def styled(status: str) -> tuple[str, str]:
        label, color = _STATUS_STYLE.get(status, (status, ""))
        return label, (color if use_color else "")

    # column widths (status uses the plain label width, colour codes excluded)
    status_w = max([len("STATUS")] + [len(styled(r.status)[0]) for r in results] or [len("STATUS")])
    case_w   = max([len("TEST CASE")] + [len(r.case) for r in results])
    desc_w   = min(60, max([len("DESCRIPTION")] + [len(r.description) for r in results]))

    def trunc(s: str, w: int) -> str:
        return s if len(s) <= w else s[: w - 1] + "…"

    sep  = f"+-{'-'*status_w}-+-{'-'*case_w}-+-{'-'*desc_w}-+"
    head = f"| {'STATUS':<{status_w}} | {'TEST CASE':<{case_w}} | {'DESCRIPTION':<{desc_w}} |"

    print()
    print("=" * len(sep))
    print(" TEST SUMMARY")
    print(sep)
    print(head)
    print(sep)
    for r in results:
        label, color = styled(r.status)
        status_cell = f"{color}{label:<{status_w}}{_RESET if color else ''}"
        desc = trunc(r.description, desc_w)
        print(f"| {status_cell} | {r.case:<{case_w}} | {desc:<{desc_w}} |")
        if r.detail:
            print(f"| {'':<{status_w}} | {'└─ ' + trunc(r.detail, case_w + desc_w):<{case_w + desc_w + 3}} |")
    print(sep)

    counts = {k: sum(1 for r in results if r.status == k) for k in _STATUS_STYLE}
    total  = len(results)
    summary = (
        f" {counts['PASS']} passed, {counts['FAIL']} failed, "
        f"{counts['COMPILE']} compile-err, {counts['NO_RESULT']} no-result, "
        f"{counts['SKIP']} skipped   ({total} total)"
    )
    print(summary)
    print("=" * len(sep))
