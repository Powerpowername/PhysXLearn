#!/usr/bin/env python3
"""UTF-8 优先、自动回退的文件读取器。

用于替代 PowerShell 5.1 的 Get-Content / Select-String —— 后者按系统 ANSI
代码页（GBK / 936）解码，读取 UTF-8 源码时中文会乱码。

用法示例:
    python read_file.py src/main.cpp
    python read_file.py src/main.cpp --lines 20:60
    python read_file.py include --glob "**/*.hpp" --stat
    python read_file.py src/main.cpp --grep "PhysX"
    python read_file.py src "Joint/src" --lines -40
"""
from __future__ import annotations

import argparse
import codecs
import re
import sys
from pathlib import Path

# BOM 判定表。顺序有讲究：UTF-32 LE 的 BOM 以 UTF-16 LE 的 BOM 开头，必须先判 UTF-32。
# 必须显式比对 BOM 字节，不能只靠"解码是否成功"来判断 ——
# Python 的 utf-8-sig 解码器对「没有 BOM 的 UTF-8 文件」同样会成功，
# 于是所有 UTF-8 文件都会被误报成 utf-8-sig，掩盖掉"缺 BOM"这个真问题。
_BOMS = (
    (codecs.BOM_UTF8, "utf-8-sig", "utf-8 BOM"),
    (codecs.BOM_UTF32_LE, "utf-32", "utf-32 LE BOM"),
    (codecs.BOM_UTF32_BE, "utf-32", "utf-32 BE BOM"),
    (codecs.BOM_UTF16_LE, "utf-16", "utf-16 LE BOM"),
    (codecs.BOM_UTF16_BE, "utf-16", "utf-16 BE BOM"),
)

# 无 BOM 时的回退顺序
ENCODINGS = ("utf-8", "gbk", "latin-1")


def decode(raw: bytes) -> tuple[str, str]:
    """返回 (文本, 实际使用的编码名)。"""
    for bom, enc, label in _BOMS:
        if raw.startswith(bom):
            try:
                return raw.decode(enc), label
            except UnicodeDecodeError:
                break

    for enc in ENCODINGS:
        try:
            return raw.decode(enc), enc
        except (UnicodeDecodeError, LookupError):
            continue

    return raw.decode("utf-8", errors="replace"), "utf-8(replace)"


def parse_range(spec: str | None, total: int) -> tuple[int, int]:
    """解析 '20:60' 形式的行范围，任一端可省略，支持负数表示倒数。"""
    if not spec:
        return 1, total
    first, _, last = spec.partition(":")

    def resolve(token: str, default: int) -> int:
        token = token.strip()
        if not token:
            return default
        value = int(token)
        return total + value + 1 if value < 0 else value

    lo = resolve(first, 1)
    hi = resolve(last, total)
    return max(lo, 1), min(hi, total)


def main() -> int:
    # 显式 UTF-8 输出，避免中文在重定向 / 管道中被终端降级为乱码
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")  # type: ignore[union-attr]

    parser = argparse.ArgumentParser(description="读取文件内容（UTF-8 优先，自动回退）")
    parser.add_argument("paths", nargs="+", help="文件或目录路径")
    parser.add_argument("--lines", help="行范围，如 20:60、20:、:60、-40")
    parser.add_argument("--glob", default="**/*", help="输入目录时使用的 glob，默认 **/*")
    parser.add_argument("--stat", action="store_true", help="只输出编码 / 行数 / 字节数")
    parser.add_argument("--grep", help="只输出匹配该正则的行（忽略大小写）")
    args = parser.parse_args()

    targets: list[Path] = []
    for raw_path in args.paths:
        path = Path(raw_path)
        if path.is_dir():
            targets.extend(sorted(p for p in path.glob(args.glob) if p.is_file()))
        elif path.is_file():
            targets.append(path)
        else:
            print(f"!! 未找到: {path}", file=sys.stderr)

    if not targets:
        print("!! 没有可读取的文件", file=sys.stderr)
        return 1

    pattern = re.compile(args.grep, re.IGNORECASE) if args.grep else None
    exit_code = 0

    for path in targets:
        try:
            raw = path.read_bytes()
        except OSError as exc:
            print(f"!! 读取失败: {path} ({exc})", file=sys.stderr)
            exit_code = 1
            continue

        text, enc = decode(raw)
        rows = text.splitlines()
        print(f"===== {path}  [{enc}, {len(rows)} lines, {len(raw)} bytes] =====")

        if args.stat:
            continue

        lo, hi = parse_range(args.lines, len(rows))
        for number in range(lo, hi + 1):
            row = rows[number - 1]
            if pattern and not pattern.search(row):
                continue
            print(f"{number:>5} | {row}")

    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
