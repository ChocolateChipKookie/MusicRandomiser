// MusicRandomiser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <dirent.h>
#include <cctype>
#include <string>
#include <algorithm>
#include <list>
#include <ctime>

#define SHUFFLE				1
#define UNSHUFFLE			2
#define WRONG_NR_OF_FILES	3
#define	ERROR_OPENING_FILE	4
#define	WRONG_NAMES			5

//CHECKS IF FILENAME HAS A TAG
bool isTagged(std::string songName)
{
	if (songName[0] != '(')return false;
	bool hasNumber{ false };
	int i{ 1 };
	
	while (songName[i] && songName[i]<='9' && songName[i] >= '0')
	{
		i++;
		hasNumber = true;
	}
	if (songName[i] && songName[i + 1] && hasNumber && songName[i] == '.' && songName[i + 1] == ')')return true;
	return false;
}

//CHECKS IF FILE IS AN MP3 FILE
bool isMP3(std::string songName)
{
	return songName.find(".mp3\0")!=std::string::npos;
}

//REMOVES TAG
std::string removeTag(std::string songName)
{
	if (!isTagged(songName))return songName;
	
	int i{ 0 };
	while (songName[i++] != ')');

	return songName.substr(i);
}

//ADDS A TAG WITH THE GIVEN NUMBER
std::string addTag(int number, const char* songName)
{
	std::string result{ "(" };
	result.append(std::to_string(number));
	result.append(1, '.');
	result.append(1, ')');
	result.append(songName);
	return result;
}

//SWAPS INTS ON GIVEN LOCATIONS
void swapInts(int* first, int* second)
{
	const int helper = *first;
	*first = *second;
	*second = helper;
}

//GETS STATE OF THE *.EXE FILE
int getState()
{
	int exeFiles{ 0 };
	std::string finalFileName;
	DIR *dir;
	struct dirent *ent;


	if ((dir = opendir(".")) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			std::string fileName = static_cast<std::string>(ent->d_name);
			if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".exe")
			{
				exeFiles++;
				if (exeFiles > 1)return WRONG_NR_OF_FILES;
				finalFileName = fileName;
			}
		}
	}
	else
	{
		perror("Couldn't open directory!");
		return ERROR_OPENING_FILE;
	}

	if (exeFiles != 1)
	{
		return WRONG_NR_OF_FILES;
	}

	std::transform(finalFileName.begin(), finalFileName.end(), finalFileName.begin(), tolower);

	if (finalFileName == "shuffle.exe")return SHUFFLE;
	if (finalFileName == "unshuffle.exe")return UNSHUFFLE;
	return WRONG_NAMES;
}

int main()
{
	int state = getState();
	srand(static_cast<unsigned int>(time(NULL)));

	switch(state)
	{
	case SHUFFLE:
		{
		DIR * dir;
		struct dirent *ent;
		std::list<std::string> songNames;

		if ((dir = opendir(".")) != NULL) {

			std::list<std::string> files;

			//COPY FILE NAMES TO LIST
			while ((ent = readdir(dir)) != NULL) {
				files.push_back(static_cast<std::string>(ent->d_name));
			}
			closedir(dir);

			for (std::string file : files)
			{
				if (isMP3(file))
				{
					//ADD NAMES TO SONG NAMES LIST
					if (isTagged(file))
					{
						//IF ALREADY TAGGED, REMOVE TAG AND ADD NAME
						std::string name = removeTag(file);
						rename(file.c_str(), name.c_str());
						songNames.push_back(name);
					}
					else
					{
						songNames.push_back(static_cast<std::string>(file));
					}
				}
			}

			const int listSize = songNames.size();
			int* randomNumbers = new int[listSize];

			//RANDOMISE NUMBERS FOR TAGS
			for (int i = 0; i<listSize; i++)
			{
				randomNumbers[i] = i + 1;
			}
			for (int i = 0; i<listSize; i++)
			{
				swapInts(&(randomNumbers[i]), &(randomNumbers[rand() % listSize]));
			}

			//ADD NEW TAGS
			int i{ 0 };
			for (std::string songName : songNames)
			{
				rename(songName.c_str(), addTag(randomNumbers[i++], songName.c_str()).c_str());
			}

		}
		else {
			perror("Couldn't open directory!");
			return EXIT_FAILURE;
		}
		break;
		}
	case UNSHUFFLE:
		{
		DIR * dir;
		struct dirent *ent;

		if ((dir = opendir(".")) != NULL) {

			std::list<std::string> files;

			//COPY FILE NAMES TO LIST
			while ((ent = readdir(dir)) != NULL) {
				files.push_back(static_cast<std::string>(ent->d_name));
			}
			closedir(dir);

			//CLEAR TAGS IF FILES ARE MP3 AND TAGGED
			for (std::string file : files)
			{
				if (isMP3(file) && isTagged(file))
				{
					rename(file.c_str(), removeTag(file).c_str());
				}
			}
		}
		else {
			perror("Couldn't open directory!");
			return EXIT_FAILURE;
		}
		break;
		}
	case WRONG_NR_OF_FILES:
		{
		perror("Only one *.exe file is allowed inside the directory!");
		int i;
		std::cin >> i;
		return EXIT_FAILURE;
		}
	case WRONG_NAMES:
		{
		perror("Application has to be called shuffle or unshuffle!");
		int i;
		std::cin >> i;
		return EXIT_FAILURE;
		}
	case ERROR_OPENING_FILE:
		{
		perror("Couldn't open directory!");
		return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}

