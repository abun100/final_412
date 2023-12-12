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

