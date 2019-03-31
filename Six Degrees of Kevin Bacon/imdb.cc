#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
using namespace std;
const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
imdb::imdb(const string& directory) {
    const string actorFileName = directory + "/" + kActorFileName;
    const string movieFileName = directory + "/" + kMovieFileName;  
    actorFile = acquireFileMap(actorFileName, actorInfo);
    movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const {
    return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

imdb::~imdb() {
    releaseFileMap(actorInfo);
    releaseFileMap(movieInfo);
}

bool imdb::getCredits(const string& player, vector<film>& films) const {
    char *cur = (char*)actorFile;
    //create a char pointer to the start of the actorfile
    int size = *(int*)actorFile;
    //read in the size of all the actors
    int rt = size;
    //set the right bound
    int *offset = lower_bound((int*)(cur+sizeof(int)),(int*)(cur+(rt+1)*sizeof(int)),player,
	[&](int off, const string& actor)->bool{ 
		char *actor_name = cur+off;
		string name = actor_name; 
		return name.compare(player) < 0; });
    //rewrite the lambda function
    string found = cur + *offset;
    if (found.compare(player)!=0){ 
	return false;
    //did not find the player
    }
    char *player_name = (char*)(cur + *offset);
    //jump to the specific player that has been found
    size_t name_length = strlen(player_name);
    if (name_length%2!=0){
	name_length++;
	//add 1 extra 0 if the length of the name is not odd
    }else{
	name_length += 2;
	//add 2 extra 0s if the length of the name is not odd
    }
    short movie_nums = *(short*)(player_name+name_length);
    name_length += 2;
    //add the length of the short
    if (name_length%4!=0){
	name_length += 2;
    //make the length to the multiple of 4
    }
    for(int i=0;i<movie_nums;i++){
	int offset = *(int*)(player_name+name_length+i*sizeof(int));
	char *movie_name = (char*)movieFile+offset;
	//jump to the payload where the movies of this actor are stored
	size_t movie_len = strlen(movie_name);
	int year = *((unsigned char*)(movie_name+movie_len+1))+1900;
	struct film fm;
	fm.title = movie_name;
	fm.year = year;
	//add the film into the vector
	films.push_back(fm);
    }
    return true; 
}

bool imdb::getCast(const film& movie, vector<string>& players) const {
    //create a char pointer to the start of the moviefile
    char *cur = (char*)movieFile;
    int size = *(int*)movieFile;
    int rt = size;
    //set the right bound
    int *off = lower_bound((int*)cur+1,(int*)cur+(rt+1),movie,
        [&](int begin, const film& movie)->bool{
                char *mv_name = cur+begin;
                string name = mv_name;
		int year = *((unsigned char*)(mv_name+1+name.length()))+1900;
		//construct a film struct in the lambda function
		struct film fm;
		fm.title = name;
		fm.year = year;
                return fm < movie; });
    //rewrite the lambda function
    string found = cur + *off;
    if (found.compare(movie.title)!=0){
        return false;
    }
    //did not find the movie
    char *movie_name = cur + *off;
    //jump to the specific player that has been found
    size_t name_length = movie.title.length()+2;
    if (name_length%2!=0){
        name_length++;
	//add 1 extra 0 if the length of the name is not odd
    }
    short player_nums = *(short*)(movie_name+name_length);
    name_length+=2;
    //add the length of the short
    if (name_length%4!=0){
        name_length += 2;
	//make the length to the multiple of 4
    }
    for(int i=0;i<player_nums;i++){
        int offset = *(int*)(movie_name+name_length+i*sizeof(int));
        char *player_name = (char*)actorFile + offset;
	//jump to the payload where the actors of this movie are stored
	string actor_name= player_name;
	players.push_back(actor_name);
    } 
    return true; 
}

const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info) {
    struct stat stats;
    stat(fileName.c_str(), &stats);
    info.fileSize = stats.st_size;
    info.fd = open(fileName.c_str(), O_RDONLY);
    return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info) {
    if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
    if (info.fd != -1) close(info.fd);
}
