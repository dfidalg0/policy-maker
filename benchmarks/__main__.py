import numpy as np
import subprocess as sp
from matplotlib import pyplot as plt
from pathlib import Path

ROOT = Path(__file__).parent.parent


def reject_outliers(data, m=2.):
    d = np.abs(data - np.median(data))
    mdev = np.median(d)
    s = d/mdev if mdev else 0.
    return data[s < m]


def run_benchmark(name: str) -> None:
    dir = ROOT / 'benchmarks' / name

    print('Compiling filter...')

    sp.run([
        ROOT / 'bin' / 'compiler',
        dir / 'main.pol',
        '-o', 'filter.cc'
    ], cwd=dir, check=True)

    print('Compiling benchmark...')

    sp.run([
        'g++', '-std=c++17', '-o', 'main', '../main.cc', 'main.cc', 'filter.cc'
    ], cwd=dir, check=True)

    print('Running benchmark...')

    proc = sp.run(['./main'], cwd=dir, stdout=sp.DEVNULL, stderr=sp.PIPE)

    nofilter, filter = (np.array([
        int(n) for n in output.strip().split('\n')
    ]) for output in proc.stderr.decode().split('---'))

    filter = reject_outliers(filter)
    nofilter = reject_outliers(nofilter)

    fmean = filter.mean()
    nmean = nofilter.mean()

    fstd = filter.std()
    nstd = nofilter.std()

    print('Sem filtro: {:.2f} ± {:.2f} ns'.format(nmean, nstd))
    print('Com filtro: {:.2f} ± {:.2f} ns'.format(fmean, fstd))

    fig, axis = plt.subplots(1, 2, tight_layout=True, figsize=(10, 5))

    axis[0].hist(nofilter)
    axis[0].set_title('Sem filtro')
    axis[0].set_xlabel('Tempo (ns)')
    axis[0].set_ylabel('Frequência')
    nlim = axis[0].get_ylim()
    axis[0].vlines(nmean, *nlim, color='red')
    axis[0].vlines(nmean + nstd, *nlim, color='red', linestyle='dotted')
    axis[0].vlines(nmean - nstd, *nlim, color='red', linestyle='dotted')
    axis[0].set_ylim(*nlim)

    axis[1].hist(filter)
    axis[1].set_title('Com filtro')
    axis[1].set_xlabel('Tempo (ns)')
    axis[1].set_ylabel('Frequencia')
    flim = axis[1].get_ylim()
    axis[1].vlines(fmean, *flim, color='red')
    axis[1].vlines(fmean + fstd, *flim, color='red', linestyle='dotted')
    axis[1].vlines(fmean - fstd, *flim, color='red', linestyle='dotted')
    axis[1].set_ylim(*flim)

    fig.suptitle(name)

    plt.show()


if __name__ == "__main__":
    from sys import argv

    run_benchmark(argv[1])
