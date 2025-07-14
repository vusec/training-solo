import argparse
from elftools.elf.elffile import ELFFile
import sys


def main(binary, start, end, output):

    with open(binary, "rb") as f:
        elf = ELFFile(f)
        for segment in elf.iter_segments():
            if segment['p_type'] != 'PT_LOAD':
                continue
            seg_start = segment['p_vaddr']
            seg_end = seg_start + segment['p_memsz']
            # print(f"{seg_start:x} - {seg_end:x}")
            if start >= seg_start and end <= seg_end:
                offset = segment['p_offset'] + (start - seg_start)
                size = end - start
                f.seek(offset)
                data = f.read(size)
                print(data)
            #     # with open("output.bin", "wb") as out:
            #         # out.write(data)
            #     print(f"Extracted {size} bytes from 0x{start:x} to 0x{end:x} into output.bin")
            #     break


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('binary')
    arg_parser.add_argument('start')
    arg_parser.add_argument('end')
    arg_parser.add_argument('output')

    args = arg_parser.parse_args()
    main(args.binary, int(args.start, 16), int(args.end, 16), args.output)
