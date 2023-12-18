//
//  main.c
//  Final Project CSC412
//
//  Created by Jean-Yves Hervé on 2020-12-01, rev. 2023-12-04
//
//	This is public domain code.  By all means appropriate it and change is to your
//	heart's content.

#include <iostream>
#include <string>
#include <random>
#include <thread>
#include <chrono>
#include <queue>
//
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <set>
#include <mutex>
//
#include "gl_frontEnd.h"

//	feel free to "un-use" std if this is against your beliefs.
using namespace std;

#if 0
//-----------------------------------------------------------------------------
#pragma mark -
#pragma mark Private Functions' Prototypes
//-----------------------------------------------------------------------------
#endif

typedef struct
{
	thread thread_process;
	unsigned int index;
	bool keepGoing;
    bool stuck;
    std::set<std::pair<int, int>> visitedSquares;
} ThreadInfo;

struct partitionStatus
{	
	// If a partition bounces back and forward, stop the thread 
	unsigned int numMoves;

	// Determines if a partition cannot move in a certain direction
	bool leftStuck;
	bool rightStuck;
	bool upStuck;
	bool downStuck;
};

void initializeApplication(void);
GridPosition getNewFreePosition(void);
Direction newDirection(Direction forbiddenDir = Direction::NUM_DIRECTIONS);
TravelerSegment newTravelerSegment(const TravelerSegment& currentSeg, bool& canAdd);
void generateWalls(void);
void generatePartitions(void);
void drawGrid(void);

void initializeUserData(int argc, char* argv[]);
void initializeThreads();
TravelerSegment handleObstacleCase(TravelerSegment& currentSeg, int travelIndex, int i);
Direction getOppositeDir(const Direction& dir);
void* moveTraveler(ThreadInfo*);
void togglePauseDrawing();
void join_threads(ThreadInfo* info, int numTravelers);
std::vector<int> getAvailableDirections(TravelerSegment& currentSeg, int travelIndex);
int findPartition(const TravelerSegment& currentSeg, bool isVertical);
void movePartition(int index);
void isPartitionStuck(int index);
Direction getNewDir(int dir);



#if 0
//-----------------------------------------------------------------------------
#pragma mark -
#pragma mark Application-level Global Variables
//-----------------------------------------------------------------------------
#endif

//	Don't rename any of these variables
//-------------------------------------
//	The state grid and its dimensions (arguments to the program)
SquareType** grid;
unsigned int numRows = 0;	//	height of the grid
unsigned int numCols = 0;	//	width
unsigned int numTravelers = 0;	//	initial number
unsigned int numTravelersDone = 0;
unsigned int numLiveThreads = 0;		//	the number of live traveler threads
vector<Traveler> travelerList;
vector<SlidingPartition> partitionList;
vector<partitionStatus> partStatus; //	Determines if partition can move or not
GridPosition	exitPos;	//	location of the exit

//	travelers' sleep time between moves (in microseconds)
const int MIN_SLEEP_TIME = 1000;
int travelerSleepTime = 100000;

//	An array of C-string where you can store things you want displayed
//	in the state pane to display (for debugging purposes?)
//	Dont change the dimensions as this may break the front end
const int MAX_NUM_MESSAGES = 8;
const int MAX_LENGTH_MESSAGE = 32;
char** message;
time_t launchTime;

//	Random generators:  For uniform distributions
const unsigned int MAX_NUM_INITIAL_SEGMENTS = 6;
random_device randDev;
default_random_engine engine(randDev());
uniform_int_distribution<unsigned int> unsignedNumberGenerator(0, numeric_limits<unsigned int>::max());
uniform_int_distribution<unsigned int> segmentNumberGenerator(0, MAX_NUM_INITIAL_SEGMENTS);
uniform_int_distribution<unsigned int> segmentDirectionGenerator(0, static_cast<unsigned int>(Direction::NUM_DIRECTIONS)-1);
uniform_int_distribution<unsigned int> headsOrTails(0, 1);
uniform_int_distribution<unsigned int> rowGenerator;
uniform_int_distribution<unsigned int> colGenerator;


// Our Defined varaibles
ThreadInfo* info;
int growSegment = 5;
vector<int> counters;
bool pauseDrawing = false;
int numTravelersSolved = 0;
mutex myLock;

<<<<<<< HEAD
//Partition Max Moves Threshold 
unsigned int MAX_PARTITION_THRESHOLD = 1000;

=======
const int dr[] = {1, 0, -1, 0};
const int dc[] = {0, -1, 0, 1};
>>>>>>> dcb262b046bdbb401044deae45fd11c142c02b77

#if 0
//-----------------------------------------------------------------------------
#pragma mark -
#pragma mark Functions called by the front end
//-----------------------------------------------------------------------------
#endif
//==================================================================================
//	These are the functions that tie the simulation with the rendering.
//	Some parts are "don't touch."  Other parts need your intervention
//	to make sure that access to critical section is properly synchronized
//==================================================================================

void drawTravelers(void)
{
    for (unsigned int k = 0; k < travelerList.size(); k++)
    {	
		if (pauseDrawing) {
			// Pause drawing
        	drawTraveler(travelerList[k]);
        	return;
		}
		myLock.lock();
        // Draw the traveler at the updated position
        drawTraveler(travelerList[k]);
		myLock.unlock();
    }

    // if travelers done is equal to the number of travelers, then we are done




    // Uncomment if you want to add delay
    // this_thread::sleep_for(chrono::milliseconds(250));
}

void updateMessages(void)
{
	//	Here I hard-code a few messages that I want to see displayed
	//	in my state pane.  The number of live robot threads will
	//	always get displayed.  No need to pass a message about it.
	unsigned int numMessages = 4;
	sprintf(message[0], "We created %d travelers", numTravelers);
	sprintf(message[1], "%d travelers solved the maze", numTravelersSolved);
	sprintf(message[2], "%d travelers got stuck", numTravelersDone-numTravelersSolved);
	sprintf(message[3], "Simulation run time: %ld s", time(NULL)-launchTime);
	
	//---------------------------------------------------------
	//	This is the call that makes OpenGL render information
	//	about the state of the simulation.
	//
	//	You *must* synchronize this call.
	//---------------------------------------------------------
	drawMessages(numMessages, message);
}

void togglePauseDrawing()
{
    pauseDrawing = !pauseDrawing;
}

void handleKeyboardEvent(unsigned char c, int x, int y)
{
	int ok = 0;

	switch (c)
	{
		//	'esc' to quit
		case 27:
            join_threads(info, numTravelers);
			exit(0);
			break;

		//	slowdown
		case ',':
			slowdownTravelers();
			ok = 1;
			break;

		//	speedup
		case '.':
			speedupTravelers();
			ok = 1;
			break;
		// Toggle pause drawing when 'p' key is pressed
        case 'p':
            togglePauseDrawing();
			join_threads(info, numTravelers);
            ok = 1;
            break;

		default:
			ok = 1;
			break;
	}
	if (!ok)
	{
		//	do something?
	}
}


//------------------------------------------------------------------------
//	You shouldn't have to touch this one.  Definitely if you don't
//	add the "producer" threads, and probably not even if you do.
//------------------------------------------------------------------------
void speedupTravelers(void)
{
	//	decrease sleep time by 20%, but don't get too small
	int newSleepTime = (8 * travelerSleepTime) / 10;
	
	if (newSleepTime > MIN_SLEEP_TIME)
	{
		travelerSleepTime = newSleepTime;
	}
}

void slowdownTravelers(void)
{
	//	increase sleep time by 20%.  No upper limit on sleep time.
	//	We can slow everything down to admistrative pace if we want.
	travelerSleepTime = (12 * travelerSleepTime) / 10;
}




//------------------------------------------------------------------------
//	You shouldn't have to change anything in the main function besides
//	initialization of the various global variables and lists
//------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	//	We know that the arguments  of the program  are going
	//	to be the width (number of columns) and height (number of rows) of the
	//	grid, the number of travelers, etc.
	//	So far, I hard code-some values
	numRows = 20;
	numCols = 20;
	numTravelers = 1;
	numLiveThreads = 0;
	numTravelersDone = 0;

	//	Even though we extracted the relevant information from the argument
	//	list, I still need to pass argc and argv to the front-end init
	//	function because that function passes them to glutInit, the required call
	//	to the initialization of the glut library.
	initializeFrontEnd(argc, argv);

	// Initialize user inputted data
	initializeUserData(argc, argv);
	
	//	Now we can do application-level initialization
	initializeApplication();

	// Initialize the threads
	initializeThreads();

	launchTime = time(NULL);

	//	Now we enter the main loop of the program and to a large extend
	//	"lose control" over its execution.  The callback functions that 
	//	we set up earlier will be called when the corresponding event
	//	occurs
	glutMainLoop();
	
	//	Free allocated resource before leaving (not absolutely needed, but
	//	just nicer.  Also, if you crash there, you know something is wrong
	//	in your code.
	for (unsigned int i=0; i< numRows; i++)
		delete []grid[i];
	delete []grid;
	for (int k=0; k<MAX_NUM_MESSAGES; k++)
		delete []message[k];
	delete []message;
	
	//	This will probably never be executed (the exit point will be in one of the
	//	call back functions).
	return 0;

}

void initializeUserData(int argc, char* argv[])  
{
	numRows = stoi(argv[1]);
	numCols = stoi(argv[2]);
	numTravelers = stoi(argv[3]);
	growSegment = stoi(argv[4]);
}
//==================================================================================
//
//	This is a function that you have to edit and add to.
//
//==================================================================================


void initializeApplication(void)
{
	//	Initialize some random generators
	rowGenerator = uniform_int_distribution<unsigned int>(0, numRows-1);
	colGenerator = uniform_int_distribution<unsigned int>(0, numCols-1);

	//	Allocate the grid
	grid = new SquareType*[numRows];
	for (unsigned int i=0; i<numRows; i++)
	{
		grid[i] = new SquareType[numCols];
		for (unsigned int j=0; j< numCols; j++)
			grid[i][j] = SquareType::FREE_SQUARE;
		
	}

	message = new char*[MAX_NUM_MESSAGES];
	for (unsigned int k=0; k<MAX_NUM_MESSAGES; k++)
		message[k] = new char[MAX_LENGTH_MESSAGE+1];
		
	//---------------------------------------------------------------
	//	All the code below to be replaced/removed
	//	I initialize the grid's pixels to have something to look at
	//---------------------------------------------------------------
	//	Yes, I am using the C random generator after ranting in class that the C random
	//	generator was junk.  Here I am not using it to produce "serious" data (as in a
	//	real simulation), only wall/partition location and some color
	srand((unsigned int) time(NULL));

	//	generate a random exit
	exitPos = getNewFreePosition();
	grid[exitPos.row][exitPos.col] = SquareType::EXIT;

	//	Generate walls and partitions
	generateWalls();
	generatePartitions();
	
	//	Initialize traveler info structs
	//	You will probably need to replace/complete this as you add thread-related data
	float** travelerColor = createTravelerColors(numTravelers);
	for (unsigned int k=0; k<numTravelers; k++) {
		GridPosition pos = getNewFreePosition();
		//	Note that treating an enum as a sort of integer is increasingly
		//	frowned upon, as C++ versions progress
		Direction dir = static_cast<Direction>(segmentDirectionGenerator(engine));

		TravelerSegment seg = {pos.row, pos.col, dir};
		Traveler traveler;
		traveler.segmentList.push_back(seg);
		grid[pos.row][pos.col] = SquareType::TRAVELER;

        //    I add 0-n segments to my travelers
        unsigned int numAddSegments = segmentNumberGenerator(engine);
        TravelerSegment currSeg = traveler.segmentList[0];
        bool canAddSegment = true;
        cout << "Traveler " << k << " at (row=" << pos.row << ", col=" <<
        pos.col << "), direction: " << dirStr(dir) << ", with up to " << numAddSegments << " additional segments" << endl;
        cout << "\t";

        for (unsigned int s=0; s<numAddSegments && canAddSegment; s++){
            TravelerSegment newSeg = newTravelerSegment(currSeg, canAddSegment);
            if (canAddSegment){
                traveler.segmentList.push_back(newSeg);
                grid[newSeg.row][newSeg.col] = SquareType::TRAVELER;
                currSeg = newSeg;
                cout << dirStr(newSeg.dir) << "  ";
            }
        }
        cout << endl;

		for (unsigned int c=0; c<4; c++)
			traveler.rgba[c] = travelerColor[k][c];
		
		travelerList.push_back(traveler);
	}

	//	free array of colors
	for (unsigned int k=0; k<numTravelers; k++)
		delete []travelerColor[k];
	delete []travelerColor;

	for(int i =0 ; i<partitionList.size(); i++) {
		cout << "part: " << i << " LIST: "; 
		for(auto& block : partitionList[i].blockList) {
			cout << " " << block.row << " " << block.col;
		}
		cout << endl;
	}	
	for(int j=0; j<partitionList.size(); j++) {
		partStatus.push_back({0,false, false, false, false});
	}
}

void initializeThreads() {
	//Create our Array of ThreadInfo objects based on number of travelers
	info = (ThreadInfo*) calloc(numTravelers, sizeof(ThreadInfo));

	//Create our thread objects
	for (unsigned k=0; k < numTravelers; k++)
	{
		counters.push_back(0);
		info[k].index = k;
		info[k].thread_process = thread(moveTraveler, &info[k]);
		info[k].keepGoing = true;
        info[k].stuck = false;
        info[k].visitedSquares = {};
	}
}

//------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Generation Helper Functions
#endif
//------------------------------------------------------

GridPosition getNewFreePosition(void)
{
	GridPosition pos;

	bool noGoodPos = true;
	while (noGoodPos)
	{
		unsigned int row = rowGenerator(engine);
		unsigned int col = colGenerator(engine);
		if (grid[row][col] == SquareType::FREE_SQUARE)
		{
			pos.row = row;
			pos.col = col;
			noGoodPos = false;
		}
	}
	return pos;
}

Direction newDirection(Direction forbiddenDir)
{
	bool noDir = true;

	Direction dir = Direction::NUM_DIRECTIONS;
	while (noDir)
	{
		dir = static_cast<Direction>(segmentDirectionGenerator(engine));
		noDir = (dir==forbiddenDir);
	}
	return dir;
}


TravelerSegment newTravelerSegment(const TravelerSegment& currentSeg, bool& canAdd)
{
	TravelerSegment newSeg;
	switch (currentSeg.dir)
	{
		case Direction::NORTH:
			if (	currentSeg.row < numRows-1 &&
					grid[currentSeg.row+1][currentSeg.col] == SquareType::FREE_SQUARE)
			{
				newSeg.row = currentSeg.row+1;
				newSeg.col = currentSeg.col;
				newSeg.dir = newDirection(Direction::SOUTH);
				grid[newSeg.row][newSeg.col] = SquareType::TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;

		case Direction::SOUTH:
			if (	currentSeg.row > 0 &&
					grid[currentSeg.row-1][currentSeg.col] == SquareType::FREE_SQUARE)
			{
				newSeg.row = currentSeg.row-1;
				newSeg.col = currentSeg.col;
				newSeg.dir = newDirection(Direction::NORTH);
				grid[newSeg.row][newSeg.col] = SquareType::TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;

		case Direction::WEST:
			if (	currentSeg.col < numCols-1 &&
					grid[currentSeg.row][currentSeg.col+1] == SquareType::FREE_SQUARE)
			{
				newSeg.row = currentSeg.row;
				newSeg.col = currentSeg.col+1;
				newSeg.dir = newDirection(Direction::EAST);
				grid[newSeg.row][newSeg.col] = SquareType::TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;

		case Direction::EAST:
			if (	currentSeg.col > 0 &&
					grid[currentSeg.row][currentSeg.col-1] == SquareType::FREE_SQUARE)
			{
				newSeg.row = currentSeg.row;
				newSeg.col = currentSeg.col-1;
				newSeg.dir = newDirection(Direction::WEST);
				grid[newSeg.row][newSeg.col] = SquareType::TRAVELER;
				canAdd = true;
			}
			//	no more segment
			else
				canAdd = false;
			break;
		
		default:
			canAdd = false;
	}
	
	return newSeg;
}

void generateWalls(void)
{
	const unsigned int NUM_WALLS = (numCols+numRows)/4;

	//	I decide that a wall length  cannot be less than 3  and not more than
	//	1/4 the grid dimension in its Direction
	const unsigned int MIN_WALL_LENGTH = 3;
	const unsigned int MAX_HORIZ_WALL_LENGTH = numCols / 3;
	const unsigned int MAX_VERT_WALL_LENGTH = numRows / 3;
	const unsigned int MAX_NUM_TRIES = 20;

	bool goodWall = true;
	
	//	Generate the vertical walls
	for (unsigned int w=0; w< NUM_WALLS; w++)
	{
		goodWall = false;
		
		//	Case of a vertical wall
		if (headsOrTails(engine))
		{
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodWall; k++)
			{
				//	let's be hopeful
				goodWall = true;
				
				//	select a column index
				unsigned int HSP = numCols/(NUM_WALLS/2+1);
				unsigned int col = (1+ unsignedNumberGenerator(engine)%(NUM_WALLS/2-1))*HSP;
				unsigned int length = MIN_WALL_LENGTH + unsignedNumberGenerator(engine)%(MAX_VERT_WALL_LENGTH-MIN_WALL_LENGTH+1);
				
				//	now a random start row
				unsigned int startRow = unsignedNumberGenerator(engine)%(numRows-length);
				for (unsigned int row=startRow, i=0; i<length && goodWall; i++, row++)
				{
					if (grid[row][col] != SquareType::FREE_SQUARE)
						goodWall = false;
				}
				
				//	if the wall first, add it to the grid
				if (goodWall)
				{
					for (unsigned int row=startRow, i=0; i<length && goodWall; i++, row++)
					{
						grid[row][col] = SquareType::WALL;
					}
				}
			}
		}
		// case of a horizontal wall
		else
		{
			goodWall = false;
			
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodWall; k++)
			{
				//	let's be hopeful
				goodWall = true;
				
				//	select a column index
				unsigned int VSP = numRows/(NUM_WALLS/2+1);
				unsigned int row = (1+ unsignedNumberGenerator(engine)%(NUM_WALLS/2-1))*VSP;
				unsigned int length = MIN_WALL_LENGTH + unsignedNumberGenerator(engine)%(MAX_HORIZ_WALL_LENGTH-MIN_WALL_LENGTH+1);
				
				//	now a random start row
				unsigned int startCol = unsignedNumberGenerator(engine)%(numCols-length);
				for (unsigned int col=startCol, i=0; i<length && goodWall; i++, col++)
				{
					if (grid[row][col] != SquareType::FREE_SQUARE)
						goodWall = false;
				}
				
				//	if the wall first, add it to the grid
				if (goodWall)
				{
					for (unsigned int col=startCol, i=0; i<length && goodWall; i++, col++)
					{
						grid[row][col] = SquareType::WALL;
					}
				}
			}
		}
	}
}


void generatePartitions(void)
{
	const unsigned int NUM_PARTS = (numCols+numRows)/4;

	//	I decide that a partition length  cannot be less than 3  and not more than
	//	1/4 the grid dimension in its Direction
	const unsigned int MIN_PARTITION_LENGTH = 3;
	const unsigned int MAX_HORIZ_PART_LENGTH = numCols / 3;
	const unsigned int MAX_VERT_PART_LENGTH = numRows / 3;
	const unsigned int MAX_NUM_TRIES = 20;

	bool goodPart = true;

	for (unsigned int w=0; w< NUM_PARTS; w++)
	{
		goodPart = false;
		
		//	Case of a vertical partition
		if (headsOrTails(engine))
		{
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodPart; k++)
			{
				//	let's be hopeful
				goodPart = true;
				
				//	select a column index
				unsigned int HSP = numCols/(NUM_PARTS/2+1);
				unsigned int col = (1+ unsignedNumberGenerator(engine)%(NUM_PARTS/2-2))*HSP + HSP/2;
				unsigned int length = MIN_PARTITION_LENGTH + unsignedNumberGenerator(engine)%(MAX_VERT_PART_LENGTH-MIN_PARTITION_LENGTH+1);
				
				//	now a random start row
				unsigned int startRow = unsignedNumberGenerator(engine)%(numRows-length);
				for (unsigned int row=startRow, i=0; i<length && goodPart; i++, row++)
				{
					if (grid[row][col] != SquareType::FREE_SQUARE)
						goodPart = false;
				}
				
				//	if the partition is possible,
				if (goodPart)
				{
					//	add it to the grid and to the partition list
					SlidingPartition part;
					part.isVertical = true;
					for (unsigned int row=startRow, i=0; i<length && goodPart; i++, row++)
					{
						grid[row][col] = SquareType::VERTICAL_PARTITION;
						GridPosition pos = {row, col};
						part.blockList.push_back(pos);
					}
					partitionList.push_back(part);
				}
			}
		}
		// case of a horizontal partition
		else
		{
			goodPart = false;
			
			//	I try a few times before giving up
			for (unsigned int k=0; k<MAX_NUM_TRIES && !goodPart; k++)
			{
				//	let's be hopeful
				goodPart = true;
				
				//	select a column index
				unsigned int VSP = numRows/(NUM_PARTS/2+1);
				unsigned int row = (1+ unsignedNumberGenerator(engine)%(NUM_PARTS/2-2))*VSP + VSP/2;
				unsigned int length = MIN_PARTITION_LENGTH + unsignedNumberGenerator(engine)%(MAX_HORIZ_PART_LENGTH-MIN_PARTITION_LENGTH+1);
				
				//	now a random start row
				unsigned int startCol = unsignedNumberGenerator(engine)%(numCols-length);
				for (unsigned int col=startCol, i=0; i<length && goodPart; i++, col++)
				{
					if (grid[row][col] != SquareType::FREE_SQUARE)
						goodPart = false;
				}
				
				//	if the wall first, add it to the grid and build SlidingPartition object
				if (goodPart)
				{
					SlidingPartition part;
					part.isVertical = false;
					for (unsigned int col=startCol, i=0; i<length && goodPart; i++, col++)
					{
						grid[row][col] = SquareType::HORIZONTAL_PARTITION;
						GridPosition pos = {row, col};
						part.blockList.push_back(pos);
					}
					partitionList.push_back(part);
				}
			}
		}
	}
}

std::vector<int> getAvailableDirections(TravelerSegment& currentSeg, int travelIndex){
    //We don't want to pick a direction towards the obstacle
    const int dr[] = {1, 0, -1, 0};
    const int dc[] = {0, -1, 0, 1};
    // Try to find a free space in the new direction

    TravelerSegment newSeg;
    // Define a vector to store the available directions
    std::vector<int> availableDirections;

    // Define our bool to check if we found an unvisited square
    bool foundUnvisitedSquare = false;

    // Add the current square to the visited squares
    info[travelIndex].visitedSquares.insert({currentSeg.row, currentSeg.col});

    for (int i = 0; i < 4; ++i) {
        // Bounds check
        if (currentSeg.row + dr[i] < 0 || currentSeg.row + dr[i] >= numRows ||
            currentSeg.col + dc[i] < 0 || currentSeg.col + dc[i] >= numCols) {
            continue;
        }

        if (grid[currentSeg.row + dr[i]][currentSeg.col + dc[i]] == SquareType::FREE_SQUARE ||
            grid[currentSeg.row + dr[i]][currentSeg.col + dc[i]] == SquareType::EXIT ||
			grid[currentSeg.row + dr[i]][currentSeg.col + dc[i]] == SquareType::HORIZONTAL_PARTITION ||
			grid[currentSeg.row + dr[i]][currentSeg.col + dc[i]] == SquareType::VERTICAL_PARTITION){

            // create a pair of the free square
            std::pair<int, int> nextSquare = {currentSeg.row + dr[i], currentSeg.col + dc[i]};
			TravelerSegment temp;
			temp.row = nextSquare.first;
			temp.col = nextSquare.second;


			 // Check the status of partitions
            if (grid[nextSquare.first][nextSquare.second] == SquareType::HORIZONTAL_PARTITION) {
				int partIndex = findPartition(temp, false);
				// cout << "Part Index: " << partIndex << endl;
				isPartitionStuck(partIndex);
                if (!partStatus[partIndex].rightStuck || !partStatus[partIndex].leftStuck) {
                    availableDirections.push_back(i);
                }
            } 
			else if (grid[nextSquare.first][nextSquare.second] == SquareType::VERTICAL_PARTITION) {
				int partIndex = findPartition(temp, true);
				// cout << "Part Index: " << partIndex << endl;
				isPartitionStuck(partIndex);
                if (!partStatus[partIndex].upStuck || !partStatus[partIndex].downStuck) {
                    availableDirections.push_back(i);
                }
			}
            // If the square is unvisited, consider it
            else if ( info[travelIndex].visitedSquares.find(nextSquare) ==  info[travelIndex].visitedSquares.end()) {
                foundUnvisitedSquare = true;
                availableDirections.push_back(i);
            }

                // If the square is visited and no unvisited square has been found, consider it
            else if (!foundUnvisitedSquare &&  info[travelIndex].visitedSquares.find(nextSquare) !=  info[travelIndex].visitedSquares.end()) {
                availableDirections.push_back(i);
            }
        }
    }

    return availableDirections;
}

TravelerSegment handleObstacleCase(TravelerSegment& currentSeg, int travelIndex, int i)
{
	TravelerSegment newSeg;

    Direction newDir = getNewDir(i);

    if(grid[currentSeg.row + dr[i]][currentSeg.col + dc[i]] == SquareType::EXIT)
    {
        newSeg = {currentSeg.row + dr[i], currentSeg.col + dc[i], newDir};
    }
    else
    {
        // Move in the new direction
        newSeg = {currentSeg.row + dr[i], currentSeg.col + dc[i], newDir};
        grid[newSeg.row][newSeg.col] = SquareType::TRAVELER;
        currentSeg = newSeg;
    }
	return newSeg;
}

Direction getOppositeDir(const Direction& dir) {
	Direction opposite;
	switch (dir)
	{
	case Direction::NORTH:
		opposite = Direction::SOUTH;
		break;
	case Direction::SOUTH:
		opposite = Direction::NORTH;
		break;
	case Direction::WEST:
		opposite = Direction::EAST;
		break;
	case Direction::EAST:
		opposite = Direction::WEST;
		break;
	default:
		break;
	}
	return opposite;
}

void* moveTraveler(ThreadInfo* travelThread) {
	// cout << travelThread->keepGoing << endl;

	while(travelThread->keepGoing) {

        int attempt = 0;
		// Pointer to traveler segment list
		vector<TravelerSegment>& segments = travelerList[travelThread->index].segmentList;

		myLock.lock();
		// Create copy of first segment and determine new segment
		TravelerSegment frontSeg = segments[0];
		TravelerSegment newSeg;
		myLock.unlock();

        travelThread->stuck = true;

        std::vector<int> availableDirections;

        while(travelThread->stuck){
            // Get the available directions
            availableDirections = getAvailableDirections(frontSeg, travelThread->index);

            if (availableDirections.size() == 0) {
                // If there are no available directions, then we are stuck
                this_thread::sleep_for(chrono::milliseconds(500));
                attempt++;
                if (attempt == 5) {
                    travelThread->keepGoing = false;
                    break;
                }

            }
            else {
                // We are not stuck
                travelThread->stuck = false;
            }
        }

		
        if(!travelThread->stuck) {
            int dir = availableDirections[rand() % availableDirections.size()];

            Direction newDir = getNewDir(dir);

            if(grid[frontSeg.row + dr[dir]][frontSeg.col + dc[dir]] == SquareType::HORIZONTAL_PARTITION) {
                TravelerSegment temp = {frontSeg.row + dr[dir], frontSeg.col + dc[dir], newDir};
                //Find the correct partion
                int partitoinIndex = findPartition(temp, false);
                // Move the partition, update traveler direction, and try to move again
                movePartition(partitoinIndex);
				// cout << partStatus[partitoinIndex].numMoves << endl;
                // if status.rightStuck or status.leftStuck is true, then we are stuck
				if(partStatus[partitoinIndex].rightStuck && partStatus[partitoinIndex].leftStuck) {
					travelThread->stuck = true;
				}
				if(partStatus[partitoinIndex].numMoves == MAX_PARTITION_THRESHOLD) {
					cout << "traveler still cant go anywhere " <<  partStatus[partitoinIndex].numMoves << endl;
					partStatus[partitoinIndex].numMoves = 0; 
					travelThread->keepGoing = false;
					break;

				}
				else {
					newSeg = {frontSeg.row + dr[dir], frontSeg.col + dc[dir], newDir};
				}
            }
            else if(grid[frontSeg.row + dr[dir]][frontSeg.col + dc[dir]] == SquareType::VERTICAL_PARTITION) {
                TravelerSegment temp = {frontSeg.row + dr[dir], frontSeg.col + dc[dir], newDir};
                //Find the correct partion
                int partitoinIndex = findPartition(temp, true);
                // Move the partition, update traveler direction, and try to move again
                movePartition(partitoinIndex);
                // if status.downStuck or status.upStuck is true, then we are stuck
				if(partStatus[partitoinIndex].upStuck && partStatus[partitoinIndex].downStuck) {
					travelThread->stuck = true;
				}
				if(partStatus[partitoinIndex].numMoves == MAX_PARTITION_THRESHOLD) {
					cout << "traveler still cant go anywhere " <<  partStatus[partitoinIndex].numMoves << endl;
					travelThread->keepGoing = false;
					partStatus[partitoinIndex].numMoves = 0; 
					break;
				}
				else {
					newSeg = {frontSeg.row + dr[dir], frontSeg.col + dc[dir], newDir};
				}
            }

            else {

                if (counters[travelThread->index] % growSegment == 0) {
                    // Condition 1: Grow Segment
                    // Store the end segment
                    myLock.lock();
                    TravelerSegment endSeg = segments.back();

                    // Move each segment in the list
                    for (int i = segments.size() - 1; i > 0; i--) {
                        segments[i] = segments[i - 1];
                    }

                    // if we're at a border or obstacle, change the new segment direciton
                    newSeg = handleObstacleCase(frontSeg, travelThread->index, dir);

                    // Keep the end segment

                    segments.push_back(endSeg);
                    myLock.unlock();

                } else {
                    myLock.lock();
                    // Keep a copy of the end
                    TravelerSegment endSeg = segments.back();

                    // Condition 2: Don't Grow Segment
                    // Move each segment in the list
                    for (int i = segments.size() - 1; i > 0; i--) {
                        segments[i] = segments[i - 1];
                    }

                    // if we're at a border or obstacle, change the new segment direciton
                    newSeg = handleObstacleCase(frontSeg, travelThread->index, dir);


                    grid[endSeg.row][endSeg.col] = SquareType::FREE_SQUARE;
                    myLock.unlock();

                }


                // Get the opposite direction of head, helps render correctly

                Direction opposite = getOppositeDir(newSeg.dir);
                newSeg.dir = opposite;

                myLock.lock();
                // Update the traveler's segment list'
                segments[0] = newSeg;
                myLock.unlock();

                //Check if we reahed the exit
                if (grid[segments[0].row][segments[0].col] == SquareType::EXIT) {
                    cout << "Exit Reached!" << endl;
                    numTravelersSolved++;
                    travelThread->keepGoing = false;
                }


                counters[travelThread->index] += 1;
                // Uncomment if you want to add delay
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
	}

    // erase the traveler from the grid from the last segment to the first
    for (auto seg : travelerList[travelThread->index].segmentList) {

        // if the space is not the exit, then free it
        if (grid[seg.row][seg.col] != SquareType::EXIT){
            grid[seg.row][seg.col] = SquareType::FREE_SQUARE;
        }

    }
    // erase the segments from the traveler
    travelerList[travelThread->index].segmentList.clear();

    // Increment the number of travelers done
    numTravelersDone++;

    if (numTravelersDone == numTravelers) {
        cout << "All travelers finished moving" << endl;
        // join all threads
        //join_threads(info, numTravelers);
    }
	return NULL;
}

void join_threads(ThreadInfo* info, int numTravelers) {
    for (int i = 0; i < numTravelers; i++) {
		info[i].keepGoing = false;
        info[i].thread_process.join();
    }
}


int findPartition(const TravelerSegment& currentSeg, bool isVertical) {
	bool isFound = true; 
	int partitionIndex; 
	for(int i=0; i<partitionList.size() && isFound; i++)
	{	
		// Search correct partition direction 
		if(partitionList[i].isVertical == isVertical) 
		{
			for(int j=0; j<partitionList[i].blockList.size(); j++) 
			{
				if(currentSeg.row == partitionList[i].blockList[j].row && currentSeg.col == partitionList[i].blockList[j].col) 
				{	
					partitionIndex = i;
					isFound = false;
					break;
				}
			}
		}
	}
	return partitionIndex;
}

void movePartition(int index)
{
	vector<GridPosition>& parts = partitionList[index].blockList;
	partitionStatus& status = partStatus[index];
	bool isVertical = partitionList[index].isVertical;

	GridPosition partFront = parts[0]; 
	GridPosition partEnd = parts[parts.size() - 1]; 
	if(isVertical) {
		if(headsOrTails(engine)) {	
			//Move down
			// cout << "moved down" << endl;
			// cout << "test: " << partEnd.row << endl;
			if(partEnd.row < numRows-1 && grid[partEnd.row+1][partEnd.col] == SquareType::FREE_SQUARE) {
				status.downStuck = false;
				status.numMoves++;
				for(int i=0; i<parts.size(); i++){
					parts[i].row += 1;
					grid[parts[i].row][parts[i].col] = SquareType::VERTICAL_PARTITION;
				}
				grid[partFront.row][partFront.col] = SquareType::FREE_SQUARE;
			}
			else {
				status.downStuck = true;
			}
		}
		else {
			// Move up
			// cout << "moved up" << endl;
			// cout << "test: " << partFront.row << endl;
			if(partFront.row > 0 && grid[partFront.row-1][partFront.col] == SquareType::FREE_SQUARE) {
				status.upStuck = false;
				status.numMoves++;
				for(int i=0; i<parts.size(); i++) {
					parts[i].row -= 1;
					grid[parts[i].row][parts[i].col] = SquareType::VERTICAL_PARTITION;
				}
				grid[partEnd.row][partEnd.col] = SquareType::FREE_SQUARE;
			}
			else {
				status.upStuck = true;
			}
		}
	}
	else {
		if(headsOrTails(engine)) {	
			// Move Right
			// cout << "moved right" << endl;
			// cout << "test: " << partEnd.col << endl;
			if(partEnd.col < numCols-1 && grid[partEnd.row][partEnd.col+1] == SquareType::FREE_SQUARE) {
				status.rightStuck = false;
				status.numMoves++;
				for(int i=0; i<parts.size(); i++) {
					parts[i].col += 1;
					grid[parts[i].row][parts[i].col] = SquareType::HORIZONTAL_PARTITION;
				}
				grid[partFront.row][partFront.col] = SquareType::FREE_SQUARE;
			}
			else {
				status.rightStuck = true;
			}

		}
		else {
			// Move Left
			// cout << "moved left" << endl;
			// cout << "test: " << partFront.col << endl;
			if(partFront.col > 0 && grid[partFront.row][partFront.col-1] == SquareType::FREE_SQUARE) {
				status.leftStuck = false;
				status.numMoves++;
				for(int i=0; i<parts.size(); i++) {
					parts[i].col -= 1;
					grid[parts[i].row][parts[i].col] = SquareType::HORIZONTAL_PARTITION;
				}
				grid[partEnd.row][partEnd.col] = SquareType::FREE_SQUARE;
			}
			else {
				status.leftStuck = true;
			}
		}
	}
}

void isPartitionStuck(int index)
{
	vector<GridPosition>& parts = partitionList[index].blockList;
	partitionStatus& status = partStatus[index];
	bool isVertical = partitionList[index].isVertical;

	GridPosition partFront = parts[0]; 
	GridPosition partEnd = parts[parts.size() - 1]; 
	if(isVertical) {

		// check if down is stuck
		if(partEnd.row < numRows-1 && grid[partEnd.row+1][partEnd.col] == SquareType::FREE_SQUARE) 
			status.downStuck = false;
		else 
			status.downStuck = true;

		// check if up is stuck
		if(partFront.row > 0 && grid[partFront.row-1][partFront.col] == SquareType::FREE_SQUARE) 
			status.upStuck = false;
		else 
			status.upStuck = true;

		// cout << "Status down " << status.downStuck << endl;
		// cout << "Status up " << status.upStuck << endl;
	}
	else {

		// check if right is stuck
		if(partEnd.col < numCols-1 && grid[partEnd.row][partEnd.col+1] == SquareType::FREE_SQUARE) 
			status.rightStuck = false;
		else 
			status.rightStuck = true;

		//check if left is stuck
		if(partFront.col > 0 && grid[partFront.row][partFront.col-1] == SquareType::FREE_SQUARE) 
			status.leftStuck = false;
		else 
			status.leftStuck = true;
		// cout << "Status left " << status.leftStuck << endl;
		// cout << "Status right " << status.rightStuck << endl;
	}
}

Direction getNewDir(int dir){
    Direction newDir;
    switch (dir)
    {
        case 0:
            newDir = Direction::NORTH;
            break;
        case 1:
            newDir = Direction::EAST;
            break;
        case 2:
            newDir= Direction::SOUTH;
            break;
        case 3:
            newDir = Direction::WEST;
            break;
    }
    return newDir;
}