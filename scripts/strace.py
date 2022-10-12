import re
import sys
from subprocess import Popen, PIPE

call_re = re.compile(r'^(?:\[pid \d+\] )?\[([\s\d]+)\]\s*([a-z0-9_]+)\(')
calls = set()

with Popen(
    ['strace', '-n', '-f', *sys.argv[1:]],
    stderr=PIPE, stdout=sys.stdout, stdin=sys.stdin,
    universal_newlines=True
) as proc:
    for line in proc.stderr:
        match = call_re.match(line)

        if not match:
            continue

        nr, call = match.groups()
        nr = int(nr)

        if call not in calls:
            calls.add(call)
            print(f'[{nr:4d}]\t{call}', file=sys.stderr)
