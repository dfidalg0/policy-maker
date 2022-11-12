import re
import sys
import os
from pathlib import Path
from subprocess import DEVNULL, CalledProcessError, check_output
from typing import TypedDict
from concurrent.futures import ThreadPoolExecutor, as_completed


ROOT = Path(__file__).parent.parent


def main():
    with open(ROOT / 'target' / 'syscalls.hh', 'w') as writer:
        writer.write('''#ifndef __SYSCALLS__
#define __SYSCALLS__

#include <errors.hh>

# if defined(__i386__)
#  include "syscalls/i386.hh"
# elif defined(__x86_64__)
#  include "syscalls/x86_64.hh"
# elif defined(__arm__)
#  include "syscalls/arm.hh"
# elif defined(__aarch64__)
#  include "syscalls/aarch64.hh"
# else
#  error "Unsupported architecture"
# endif

gen::SyscallEntry get_syscall_entry(const std::string &name) {
    auto it = syscalls.find(name);

    if (it == syscalls.end()) {
        try {
            auto nr = std::stoi(name);
            return gen::SyscallEntry({
                .nr = nr | __SYSCALL_BIT,
                .overloads = gen::SyscallOverloads(),
            });
        }
        catch (std::invalid_argument &) {
            // do nothing
        }
        catch (std::out_of_range &) {
            throw CompilerError("Syscall number out of range");
        }

        throw CompilerError("Unknown syscall: " + name);
    }

    return it->second;
}

#endif // __SYSCALLS__''')

    available_syscalls = set(
        file[:-len('.csv')] for file in os.listdir(ROOT / 'scripts' / 'syscalls')
    )

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

# if defined(__ILP32__) && defined(__x86_64__)
#  define __SYSCALL_BIT 0x40000000
# else
#  define __SYSCALL_BIT 0
# endif

namespace gen {
    struct SyscallParam {
        std::string name;
        bool pointer;
    };

    typedef std::vector<SyscallParam> SyscallParams;

    typedef std::vector<SyscallParams> SyscallOverloads;

    struct SyscallEntry {
        int nr;
        SyscallOverloads overloads;
    };
}

std::unordered_map<std::string, gen::SyscallEntry> syscalls = {
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
                    '''    {"%s", gen::SyscallEntry({
        .nr = %d | __SYSCALL_BIT,
        .overloads = gen::SyscallOverloads({\n''' %
                    ( name, nr )
                )

                if not overloads:
                    writer.write('            gen::SyscallParams(),\n')

                for overload in overloads:
                    ov_str = '            gen::SyscallParams({'

                    ov_str += ', '.join(
                        '{"%s", %s}' % (
                            param['name'],
                            'true' if param['pointer'] else 'false'
                        )
                        for param in overload
                    )

                    ov_str += '}),\n'

                    writer.write(ov_str)

                writer.write('        })\n    }) },\n')

        writer.write(
            '};\n\n#endif // __SYSCALLS_%(arch)s__' % { 'arch': arch.upper() }
        )


class Param(TypedDict):
    name: str
    type: str
    pointer: bool


def handle_overload(overload: str) -> list[Param]:
    args = [arg.strip() for arg in overload.split(',')]

    if args == ['void']:
        return []

    for i, arg in enumerate(args):
        if arg.startswith('...'):
            name = str(i + 1)
            type = 'void *'
            pointer = True
        else:
            match = re.search(r'([a-zA-Z_]\w*)(\[\d*\])?$', arg)

            if not match:
                raise ValueError(f'Could not parse argument {arg}')

            name = match.group(1)
            array = match.group(2)

            type = arg[:arg.rfind(name)].strip()

            if array is not None:
                type += ' *'

            pointer = '*' in type

        args[i] = {'name': name, 'type': type, 'pointer': pointer}

    return args


def find_syscall_args(name):
    if name == 'exit':
        name = '_exit'

    try:
        out = check_output(['man', '2', name], stderr=DEVNULL).decode('utf-8')
    except CalledProcessError:
        return []

    def_re = re.compile(r'[*\s]' + name + r'\s*\(([^\)]+)\);')

    section_re = re.compile(r'\n[A-Z]+\n')

    headers = [h.strip() for h in section_re.findall(out)]
    sections = dict(zip(headers, section_re.split(out)[1:]))

    synopsis: str = sections['SYNOPSIS']

    if '#if' in synopsis:
        preprocessed = synopsis.replace('#include', '')

        preprocessed = check_output(
            args=['gcc', '-E', '-P', '-'],
            input=preprocessed.encode('utf-8'),
            stderr=sys.stderr
        ).decode('utf-8')
    else:
        preprocessed = synopsis

    overloads: list[str] = def_re.findall(preprocessed)

    if name == 'reboot':
        overloads = [overloads[0]]

    return [handle_overload(ov) for ov in overloads]


if __name__ == '__main__':
    main()
