# wav2h

## build

cd to folder and type `make`

The resulting binary will be called wav2h

## usage

```
	./wav2h <path> [-f] [-t type] [-r factor]

	-f 	 write files instead of std::out

    -t   convert to a type - options are uint8, int16, float (default)

	-r  resample factor, eg 1 is no change (default), 2.0 is half the sample rate

```

When using the -f option it will write h files next to the specified wav files.

If <path> is a folder, then wav2h will scan the folder and make a .h file for each .wav and
make a "samples.h" which includes all of them.
