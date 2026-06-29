# RLE Compressor

RLE compression replaces runs of data with a single value and a count of its repetions. The more repetitions the data has, the better RLE works at shrinking it. This makes it ideal for simple graphics like icons, line art, games, and animations, which are full of repeated elements. For data without many repeated values, using RLE could make the file bigger.

Usage:

    rle compress [raw file name] [compressed file name]
    rle decompress [compressed file name] [raw file name]
