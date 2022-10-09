import re
import sys
from pathlib import Path
from subprocess import DEVNULL, CalledProcessError, check_output
from typing import TypedDict
from concurrent.futures import ThreadPoolExecutor, as_completed


ROOT = Path(__file__).parent.parent


def main():
    with open(ROOT / 'target' / 'syscalls.hh', 'w') as writer:
        writer.write('''#ifndef __SYSCALLS__
#define __SYSCALLS__

# ifdef __i386__
#  include "syscalls/x86_32.hh"
# elif defined(__ILP32__)
#  include "syscalls/x86_x32.hh"
# else
#  include "syscalls/x86_64.hh"
# endif


#endif // __SYSCALLS__''')

    available_syscalls = set(['x86_32', 'x86_x32', 'x86_64'])

    if len(sys.argv) < 2 or sys.argv[1] not in available_syscalls:
        print('Usage: generate_syscalls.py <arch> [arch] ...')
        print(f'Available architectures: {", ".join(available_syscalls)}')
        sys.exit(1)

    generate_arch_syscalls(sys.argv[1])


def generate_arch_syscalls(arch: str):
    SOURCE = ROOT / 'scripts' / 'syscalls' / f'{arch}.csv'

    DEST_DIR = ROOT / 'target' / 'syscalls'
    DEST_DIR.mkdir(parents=True, exist_ok=True)
    DEST = DEST_DIR / f'{arch}.hh'

    with DEST.open('w') as writer:
        writer.write('''#ifndef __SYSCALLS_%(arch)s__
#define __SYSCALLS_%(arch)s__
#include <unordered_map>
#include <string>
#include <vector>

typedef std::unordered_map<std::string, bool> syscall_params;

typedef std::vector<syscall_params> syscall_overloads;

struct syscall_entry {
    int nr;
    syscall_overloads overloads;
};

std::unordered_map<std::string, syscall_entry> syscalls = {
''' % { 'arch': arch.upper() })

        with SOURCE.open() as reader:
            reader.readline()

            all_overloads = {}

            with ThreadPoolExecutor(max_workers=10) as e:
                for line in reader:
                    nr, name = line.split(',')
                    nr = int(nr)
                    name = name.strip()

                    all_overloads[e.submit(find_syscall_args, name)] = (nr, name)

            for future in as_completed(all_overloads):
                nr, name = all_overloads[future]

                overloads = future.result()

                writer.write(
                    '''    {"%s", syscall_entry({
        .nr = %d,
        .overloads = syscall_overloads({\n''' %
                    ( name, nr )
                )

                if not overloads:
                    writer.write('            syscall_params(),\n')

                for overload in overloads:
                    ov_str = '            syscall_params({'

                    ov_str += ', '.join(
                        '{"%s", %s}' % (
                            param['name'],
                            'true' if param['pointer'] else 'false'
                        )
                        for param in overload
                    )

                    ov_str += '}),\n'

                    writer.write(ov_str)

                writer.write('''        })
    }) },\n''')

        writer.write('''};

#endif // __SYSCALLS_%(arch)s__''' % { 'arch': arch.upper() })


class Param(TypedDict):
    name: str
    type: str
    pointer: bool


def handle_overload(overload: str) -> list[Param]:
    args = [arg.strip() for arg in overload.split(',')]

    for i, arg in enumerate(args):
        if arg.startswith('...'):
            name = str(i + 1)
            type = 'void *'
            pointer = True
        else:
            match = re.search(r'[a-zA-Z_]\w*(\[\d*\])?$', arg)

            if not match:
                raise ValueError(f'Could not parse argument {arg}')

            name = match.group(0)
            array = match.group(1)

            type = arg[:arg.rfind(name)].strip()

            if array is not None:
                type += ' *'

            pointer = '*' in type

        args[i] = {'name': name, 'type': type, 'pointer': pointer}

    return args


def find_syscall_args(name):
    try:
        out = check_output(['man', '2', name], stderr=DEVNULL).decode('utf-8')
    except CalledProcessError:
        return []

    def_re = re.compile(r'[*\s]' + name + r'\s*\(([^\)]+)\);')

    section_re = re.compile(r'\n[A-Z]+\n')

    headers = [h.strip() for h in section_re.findall(out)]
    sections = dict(zip(headers, section_re.split(out)[1:]))

    synopsis = sections['SYNOPSIS']

    overloads = def_re.findall(synopsis)

    if 'void' in overloads:
        return [[]]

    return [handle_overload(ov) for ov in overloads]


if __name__ == '__main__':
    main()
