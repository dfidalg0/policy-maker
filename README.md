# Policy Maker

`policy-maker` is a language and CLI for defining Seccomp BPF rules in a higher level way. You provide a Seccomp BPF policy definition under the `.pol` format and the CLI generates a plug-and-play code that installs a Seccomp BPF filter in your application.

This project was a undergraduate thesis project at [Aeronautics Institute of Technology (ITA)](http://www.ita.br/) in Brazil. The original thesis (in Portuguese) can be found [here](http://www.bdita.bibl.ita.br/TGsDigitais/lista_resumo.php?num_tg=78867).

## Similar Projects

This project is heavilly inspired by [Kafel](https://github.com/google/kafel), a C library that aims to provide a higher level interface for defining Seccomp BPF rules. If you are looking for a C library, you should check it out.

The main difference between `policy-maker` and Kafel is that `policy-maker` is a CLI, while `Kafel` is a C library. This means that `policy-maker` aims to be easier to use and integrate with projects, but it also can be less flexible.

## Instalation

`policy-maker` is distributed as a single executable file. You can download the latest version from the [releases page](https://github.com/dfidalg0/policy-maker/releases).

Alternatively, you can build it from source. You'll need to have [`g++`](https://gcc.gnu.org/), [`make`](https://www.gnu.org/software/make/), [`python3`](https://www.python.org/), [`flex`](https://github.com/westes/flex) and [`bison`](https://www.gnu.org/software/bison/) installed. Once you have all these dependencies installed, you can build policy-maker by running:

```bash
make
```

This will build policy-maker inside the `bin` directory with the name `compiler`.

Either way, you can place the executable file anywhere in your system and rename it to whatever you want, but it's recommended to keep the name `policy-maker` and place it under a directory that is in your `PATH` environment variable (usually `/usr/bin/`).

## Usage

First, define a policy under the `.pol` format. You can find some examples under the `samples` directory. For example, the following policy will allow all syscalls except `socket`, which will terminate the process:

```
// main.pol

policy main allow or {
    terminate {
        socket,
    }
}
```

Then, run the CLI:

```bash
policy-maker ./main.pol
```

The CLI will generate a C file called `filter.c` and a header file called `filter.h`. You can then compile and link them with your application. These files expose a single function called `install_filter` that installs the Seccomp BPF filter in your application. You can call this function at any time, but it is recommended to call it as soon as possible in your application.

All those names are customizable via [CLI Options](#cli-options). For example, you can have a compiled output file named `custom_name.c` with a function named `custom_name` by running:

```bash
policy-maker ./main.pol --output custom_name.c --target custom_name
```

Alternatively, you can run a command with the specified policy applied without having to compile and link the generated files. All you need to do is pass the command arguments after all the policy-maker arguments.

For example, the following command will run `ls` with the policy defined in `main.pol` applied:

```bash
policy-maker ./main.pol ls
```

Under interactive mode, if you defined any syscall under a `notify` action, a worker process will be spawned to log all the syscalls that were notified and allow them.

Under the hood, the spawned process uses [`pidfd_open`](https://man7.org/linux/man-pages/man2/pidfd_open.2.html) and [`pidfd_getfd`](https://man7.org/linux/man-pages/man2/pidfd_getfd.2.html) to retrieve the notif file descriptor from the sandboxed process. This means that the worker process will only work if the kernel supports these syscalls.

**Important**: The interactive mode depends on `execve` being allowed. If you wish to use the interactive mode, make sure to allow `execve` in your policy.

## Policy Language 🚧

There's no documentation for the policy language yet. You can check out the samples under the `samples` directory to get an idea of how it works or get the original thesis (in Portuguese) [here](http://www.bdita.bibl.ita.br/TGsDigitais/lista_resumo.php?num_tg=78867).

## CLI Options

```
Usage: policy-maker <input>
         [--target (-t) <target>]
         [--print-analyzed-ast]
         [--print-ast]
         [--dry-run (-d)]
         [--output (-o) <output>]
         [--entry (-e) <entry>]
         [--print-bpf]
         [--help (-h)]
         [...]

Parameters:
  input ( --input, -i ): The input file to compile

  --target, -t:  Name of the generated function
  --print-analyzed-ast:  Print the analyzed AST to stdout
  --print-ast:  Print the AST to stdout
  --dry-run, -d:  Don't produce any output. Just validates the input file
  --output, -o:  The output file to write to
  --entry, -e:  The entry point of the program
  --print-bpf:  Print the final BPF code to stdout
  --help, -h:  Print this help message and exit

  [...] If present, any arguments after the compiler options will be passed to execvp after the filter is installed. No output will be generated.
```

## Contributing

I don't have any defined contribution guidelines yet. If you want to contribute, feel free to open an issue or a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/dfidalg0/policy-maker/blob/main/LICENSE.txt) file for details.
