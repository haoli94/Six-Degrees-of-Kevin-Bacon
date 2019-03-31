#include <vector>
#include <set>
#include <unordered_set>
#include <list>
#include <iostream>
#include <string>
#include "path.h"
#include "imdb.h"

using namespace std;

static const int kWrongArgumentCount = 1;
static const int kDatabaseNotFound = 2;

int main(int argc, char *argv[]) {

    if (argc != 3) {
	cerr << "Usage: " << argv[0] << " <start-actor> <end-actor>" << endl;
	return kWrongArgumentCount;
    }
    // if the arguments passed in are less than 3,prompt usage information
    imdb db(kIMDBDataDirectory);
    // create an object db
    if (!db.good()) {
	cerr << "Data directory not found!  Aborting..." << endl;
	return kDatabaseNotFound;
    }
    // if the db can not open imdb datasets, prompt error
    bool found = false;
    int depth = 0;
    // initialize the start depth which can only reach maximum 7
    // set default flag  
    string start_actor = argv[1];
    string end_actor = argv[2];
    //initialize the start and end actor
    set<film> films_seen;
    unordered_set<string> actors_seen;

    list<vector<pair<string,film>>> actor_movieStack;
    struct film fm;
    fm.title = "cs110";
    fm.year = 2018;

    vector<pair<string,film>> start_vec;
    pair<string,film> start_pair(start_actor,fm);
    start_vec.push_back(start_pair);
    actor_movieStack.push_back(start_vec);	
    //initialize the actor_movieStack to store the start actor and a random movie in a vector of pairs;
    actors_seen.insert(start_actor);
    //we have seen the start actor
    while (!actor_movieStack.empty()){
	vector<pair<string,film>> pairs_vec = actor_movieStack.front();
	string actor = pairs_vec[pairs_vec.size()-1].first;
	//start from the last actor-movie pair which is the current location 
	if (actor == end_actor){
	    found = true;
	    //if the we found the end_actor,turn the found flags to be true
	    break;
	}
	actor_movieStack.pop_front();
	//pop the first vector pair from the queue
	vector<film> films;
	if(db.getCredits(actor, films)){
	    vector<film>::iterator it_fm = films.begin();
	    while(it_fm != films.end()){
		film fm_search = *it_fm;
		it_fm++;	
		if (films_seen.count(fm_search)){
		    continue;
		}
		// if the films_seen contains the found film, continue
		vector<string> actors_search;
		if(db.getCast(fm_search, actors_search)){
		    for (unsigned int k=0;k<actors_search.size();k++){
			string actor_found = actors_search[k]; 
			if (!actors_seen.count(actor_found)){
			    actors_seen.insert(actor_found);
			    vector<pair<string, film>> branches = pairs_vec;
			    //make a copy of the current path
			    pair<string, film> new_pair(actor_found,fm_search);
			    branches.push_back(new_pair);
			    //update the copied path with possible new pairs
			    actor_movieStack.push_back(branches);
			    //push_back all the possible new pairs of actor and film in to the queue to be explored
			}
		    }

		}
		films_seen.insert(fm_search);
		//insert the found film into the films_seen set
	    }
	}
    }
    depth++;
    if (depth>=7 || actor_movieStack.empty()){
	cout << "No path between" << start_actor << "and" << end_actor << "has been found!" << endl;
	return 0;
    }else if(found == true) {
	vector<pair<string, film>> vec_found = actor_movieStack.front();
	//the path is left in the queue in the front
	pair<string, film> starter = vec_found[0];
	path path_found(starter.first);
	//create an path object to store the path
	vector<pair<string, film>>::iterator iter = vec_found.begin();
	iter++;
	while(iter != vec_found.end()){
	    path_found.addConnection(iter->second, iter->first);
	    iter++;
	}
	//used iterator to write the path
	cout << path_found << endl;
	//print out the path by overloaded operator
	return 0;
    }


    return 0;
}
