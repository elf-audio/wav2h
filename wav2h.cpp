#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;


bool loadWav(const std::string &path, std::vector<float> &buffer, int *outNumChannels, int *outSampleRate) {

	drwav infile;

	if(!drwav_init_file(&infile, path.c_str(), nullptr)) {
		printf("Can't open file %s for reading\n", path.c_str());
		return false;
	}
	if(outNumChannels) *outNumChannels = infile.fmt.channels;
	if(outSampleRate)  *outSampleRate = infile.fmt.sampleRate;

	buffer.resize(infile.totalPCMFrameCount * infile.fmt.channels);

	auto totalNumFramesRead = drwav_read_pcm_frames_f32(&infile, infile.totalPCMFrameCount, buffer.data());
		
	drwav_uninit(&infile);
	return totalNumFramesRead == infile.totalPCMFrameCount;
}



void zapGremlins(std::string &s) {
	if(s.size()>0) {
		if(s[0]!='_' && !isalpha(s[0])) {
			s = "m" + s;
		}
	}
	for(int i = 0; i < s.size(); i++) {
		if(s[i]==' ') s[i] = '_';
		if(s[i]==':') s[i] = '_';
		if(s[i]=='.') s[i] = '_';
		if(s[i]==',') s[i] = '_';
		if(s[i]==';') s[i] = '_';
		if(s[i]=='/') s[i] = '_';
		if(s[i]=='-') s[i] = '_';
		if(s[i]=='+') s[i] = '_';
		if(s[i]=='\\') s[i] = '_';
	}
}


bool wav2h(const fs::path &p, std::string &name, std::string &out) {
	std::vector<float> buff;
	int outNumChannels;
	int outSampleRate;
	if(!loadWav(p.string(), buff, &outNumChannels, &outSampleRate)) {
		return false;
	}
	name = p.stem().string();
	zapGremlins(name);
	out = "std::vector<float> " + name + " = {\n\t";
	for(int i = 0; i < buff.size(); i++) {
		if(i%4==0 && i > 0) {
			out += "\n\t";
		}
		out += std::to_string(buff[i]) + ",\t";
		
	}
	out += "\n};";
	return true;
}

void printUsage(char *appName) {
	printf("\nUsage\n\n\t%s <path> [-f]\n\n", appName);
	printf("\t-f \t write files instead of std::out\n\n");
}



bool writeStringToFile(const std::string &path, const std::string &data) {
	std::ofstream outfile(path, std::ios::out);
	if(outfile.fail()) {
		printf("writeStringToFile() open failed: %s %s\n", strerror(errno), path.c_str());
		return false;
	}
	outfile << data;
	if(outfile.fail()) {
		printf("writeStringToFile() data write failed: %s\n", strerror(errno));
		return false;
	}
	outfile.close();
	if(outfile.fail()) {
		printf("writeStringToFile() close failed: %s\n", strerror(errno));
		return false;
	}
	return true;
}

int main(int argc, char *argv[]) {
	if(argc!=2 && argc!=3) {
		printUsage(argv[0]);
		return 1;
	}
	fs::path p;
	bool writeFiles = false;

	if(argc==2) {
		p = argv[1];
	} else {
		if(std::string(argv[1])=="-f") {
			p = argv[2];
			writeFiles = true;
		} else if(std::string(argv[2])=="-f") {
			p = argv[1];
			writeFiles = true;
		} else {
			printUsage(argv[0]);
			return 1;
		}
	}

	if(!fs::exists(p)) {
		printf("No file exists at %s\n", p.string().c_str());
		return 1;
	}

	if(fs::is_regular_file(p)) {
		std::string name;
		std::string out;
		if(!wav2h(p, name, out)) {
			printf("Failed to read %s\n", p.string().c_str());
			return 1;
		}
		if(writeFiles) {
			auto path = p.parent_path() / (name + ".h");
			if(!writeStringToFile(path.string(), out)) {
				printf("Failed to write to %s\n", path.c_str());
				return 1;	
			}
			printf("Wrote to %s\n", path.c_str());
		} else {
			printf("%s\n", out.c_str());
		}

	} else if(fs::is_directory(p)) {
		
		std::vector<std::string> names;
		std::vector<std::string> filenames;

		std::vector<fs::path> samplePaths;

		for(auto &fp : fs::directory_iterator(p)) {
			if(fp.path().extension()==".wav" || fp.path().extension()==".WAV") {
				samplePaths.push_back(fp.path());
			}
		}

		std::sort(samplePaths.begin(), samplePaths.end());
			
		for( auto &fp : samplePaths) {
			std::string name;
			std::string out;
			if(!wav2h(fp, name, out)) {
				printf("Failed to read %s\n", fp.string().c_str());
			}
			filenames.push_back(fp.filename().string());
			names.push_back(name);

			if(writeFiles) {
				auto path = p / (name + ".h");
				if(!writeStringToFile(path.string(), out)) {
					printf("Failed to write to %s\n", path.c_str());
					return 1;	
				}
				printf("Wrote to %s\n", path.c_str());	
			} else {
				printf("%s\n", out.c_str());
			}
		}
		

		std::string samplesH;
		if(writeFiles) {
			// do includes
			for(const auto &n : names) {
				samplesH += "#include \"" + n + ".h\"\n";
			}
		}

		samplesH += "\n\nstd::vector<std::vector<float>*> samples = {\n";
		for(const auto &n : names) {
			samplesH += "\t&" + n + ",\n";
		}
		
		samplesH += "};\n\n";

		
		samplesH += "\n\nstd::vector<std::string> sampleNames = {\n";
		
		for(const auto &n : filenames) {
			samplesH += "\t\"" + n + "\",\n";
		}
		
		samplesH += "};\n\n";

		if(writeFiles) {
			auto path = p /  "samples.h";
			if(!writeStringToFile(path.string(), samplesH)) {
				printf("Failed to write to %s\n", path.c_str());
				return 1;	
			}
			printf("Wrote to %s\n", path.c_str());	
		} else {
			printf("%s\n", samplesH.c_str());
		}
	} else {
		printf("Don't know how to deal with that kind of path (%s)\n", p.string().c_str());
		return 1;
	}
	// printf("Wav size: %d\n", buff.size());
	return 0;
}