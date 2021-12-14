# wav2h


## build
cd to folder and type `make`

The resulting binary will be called wav2h

## usage
```
	./wav2h <path> [-f]

	-f 	 write files instead of std::out

```
When using the -f option it will write h files next to the specified wav files.

If <path> is a folder, then wav2h will scan the folder and make a .h file for each .wav and 
make a "samples.h" which includes all of them.