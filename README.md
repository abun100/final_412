# final_412

## Algorithm Version 1
```
For each segment for a traveler[i]:
  if growSegment:
    store end segmentList.back()
    set segment[n] = segment[n-1]
    move segment[0] in new direction
    pushback(store end)
  else
    set segment[n] = segment[n-1]
    move segment[0] in new direction
```
### Don't Grow Segment example
[s1, s2, s3] <br />
s3 = s2 <br />
s2 = s1 <br />
s1 = s1[row + dir][col + dir]

### Grow Segment Example
[s1, s2, s3] <br />
s4 = s3 <br />
s3 = s2 <br />
s2 = s1 <br />
s1 = s1[row + dir][col + dir] <br />
push_back(s4) <br />
<br />
updated : [s1, s2, s3, s4]

void drawTravelers(void)
{

```
void drawTravelers(void)
{
    for (unsigned int k = 0; k < travelerList.size(); k++)
    {	
		if (pauseDrawing) {
			// Pause drawing
        	drawTraveler(travelerList[k]);
        	return;
		}
		bool validSeg = true;
        vector<TravelerSegment>& segments = travelerList[k].segmentList;

        if (segments.empty())
            continue;  // Skip if the segment list is empty

        TravelerSegment frontSeg = segments[0];
        TravelerSegment newSeg;

        if (counter % growSegment == 0)
        {
            // Condition 1: Grow Segment
            // Store the end segment
            TravelerSegment endSeg = segments.back();

            // Move each segment in the list
            for (int i = segments.size() - 1; i > 0; i--)
            {
                segments[i] = segments[i - 1];
            }

            // Move the front segment in the opposite direction
            newSeg = moveInOpposite(frontSeg, validSeg);

			if(!validSeg) {
				// if we're at a border or obstacle, change the new segment direciton
				newSeg = handleObstacleCase(frontSeg);
			}
			segments.push_back(endSeg);
        }
        else
        {
            TravelerSegment endSeg = segments.back();
            // Condition 2: Don't Grow Segment
            // Move each segment in the list
            for (int i = segments.size() - 1; i > 0; i--)
            {
                segments[i] = segments[i - 1];
            }

            // Move the front segment in the opposite direction
            newSeg = moveInOpposite(frontSeg, validSeg);

			if(!validSeg) {
				// if we're at a border or obstacle, change the new segment direciton
				newSeg = handleObstacleCase(frontSeg);
			}

			grid[endSeg.row][endSeg.col] = SquareType::FREE_SQUARE;
        }

		Direction opposite = getOppositeDir(newSeg.dir);
		newSeg.dir = opposite;

        // Update the traveler's segment list'
        segments[0] = newSeg;

		for(auto seg : segments) {
			cout << "ROW: " << seg.row << " COL: " << seg.col << " " << dirStr(seg.dir) << endl;
		}
		cout << endl;

        // Draw the traveler at the updated position
        drawTraveler(travelerList[k]);

		//Check if we reahed the exit
		if(grid[segments[0].row][segments[0].col] == SquareType::EXIT) {
			cout << "Exit Reached!" << endl;
			exit(0);
		}
    }

    counter++;
    // Uncomment if you want to add delay
    // this_thread::sleep_for(chrono::milliseconds(500));
}
```
