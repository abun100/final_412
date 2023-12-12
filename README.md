# final_412

## Algorithm
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
[s1, s2, s3]
s3 = s2
s2 = s1
s1 = s1[row + dir][col + dir]

### Grow Segment Example
[s1, s2, s3]
s4 = s3
s3 = s2
s2 = s1
s1 = s1[row + dir][col + dir]
push_back(s4)

updated : [s1, s2, s3, s4]

