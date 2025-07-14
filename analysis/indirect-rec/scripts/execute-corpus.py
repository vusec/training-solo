#!/usr/bin/env python3

from pathlib import Path
import shutil
import multiprocessing as mp
import csv
import tqdm
import argparse
import subprocess
import os

binary_folder = ''
source_folder = ''
corpus_folder = ''


def compile_testcase(path : Path):

    cmd = [
        "./syz-prog2c",
        "-prog", str(path)
    ]

    c_file_path = os.path.join(source_folder,f"{path.name}.c")
    c_file = open(c_file_path, "w")

    res = subprocess.run([' '.join(cmd)], shell=True, stdout=c_file, check=False)

    c_file.close()

    bin_file_path = os.path.join(binary_folder,f"{path.name}")

    cmd = [
        "cc", c_file_path,
        "-o", bin_file_path
    ]

    res = subprocess.run([' '.join(cmd)], shell=True, check=False)



def do_compilation():

    # Prepare binary folder. Remove if exists
    bin_path = Path(binary_folder)

    if bin_path.exists() and bin_path.is_dir():
        shutil.rmtree(bin_path)

    bin_path.mkdir(parents=True, exist_ok=True)

    source_path = Path(source_folder)

    if source_path.exists() and source_path.is_dir():
        shutil.rmtree(source_path)

    source_path.mkdir(parents=True, exist_ok=True)


    pathlist = list(Path(corpus_folder).glob('**/*'))

    # import IPython
    # IPython.embed()
    print(f"Running with {args.j} jobs")

    pool = mp.Pool(processes=args.j)

    for _ in tqdm.tqdm(pool.imap_unordered(compile_testcase, pathlist), total=len(pathlist)):
        pass

def execute_testcase(path : Path):

    # cmd = [
    #     f"./{str(path)}"
    # ]

    # res = subprocess.run([' '.join(cmd)], shell=True, stdout=subprocess.DEVNULL, check=False)

    os.system(f"timeout 2s ./{str(path)} 2>&1 > /dev/null")



def do_execution():

    bin_path = Path(binary_folder)

    if not bin_path.exists():
        print("BInary path dos not exists!")
        exit(-1)

    pathlist = list(Path(binary_folder).glob('**/*'))

    print(f"Running with {args.j} jobs")

    pool = mp.Pool(processes=args.j)

    for _ in tqdm.tqdm(pool.imap_unordered(execute_testcase, pathlist), total=len(pathlist)):
        pass






if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser(description='Compile and execute syzkaller corpus')
    arg_parser.add_argument('-j', '-jobs', default=1, type=int)
    subparsers = arg_parser.add_subparsers(dest='command', help='command to run', required=True)

    compile_args = subparsers.add_parser('compile', help='Compile corpus files')
    compile_args.add_argument('corpus_folder')
    compile_args.add_argument('source_folder')
    compile_args.add_argument('binary_folder')
    compile_args.set_defaults(func=do_compilation)


    execute_args = subparsers.add_parser('execute', help='Excute corpus binaries')
    execute_args.add_argument('binary_folder')
    execute_args.set_defaults(func=do_execution)


    args = arg_parser.parse_args()

    if 'corpus_folder' in args:
        corpus_folder = args.corpus_folder
        source_folder = args.source_folder

    binary_folder = args.binary_folder

    args.func()
