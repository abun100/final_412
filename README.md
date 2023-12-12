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
