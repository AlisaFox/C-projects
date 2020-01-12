# Readers and Writers

## What:
Tried to solve the famous readers writers problem. The problem being that both reader and writer threads are not allowed access to the 
critical section at the same time. What would be the best way to be fair to both (allowing both readers and writers to have approximately 
the same wait time)?

## What's happening right now?
Currently there are two solutions. Both are longer than what you are used to seeing, as I also had to set up a system to time the processes and find 
the average completion times.

The first solution, is what is considered to be "the first readers writers problem". All the readers had to finish before the writers could go. 
Readers could occupy the critical section concurrently. It gives the readers priority. 
Here are the times for 60 readers and 30 writers :
> Reader average time is 0.000001 secs 
> Reader max time is 0.000064 secs 
> Reader min time is 0.000000 secs 
 
> Writer average time is 0.003930 secs 
> Writer max time is 11.899890 secs 
> Writer min time is 0.000000 secs 

In the second solution: I tried "the third readers writers problem". I do not think that it technically worked out, as the writers have
priority. The logic was: 
- If a writer arrives while readers are accessing the resource, it will wait until those readers free the resource, and then modify it.  
- New readers arriving in the meantime will have to wait.

The times I got were 
>  Reader average time is 0.110264 secs 
>  Reader max time is 2.526340 secs 
>  Reader min time is 0.000000 secs 
 
> Writer average time is 0.003983 secs 
> Writer max time is 2.017298 secs 
> Writer min time is 0.000000 secs 

So things are a bit more fair, but still there is starvation evident in the readers. The biggest differences were the maximum times.

## To-do

I need to figure out if there is an error in my code somewhere. Otherwise, I should add a timer:
after a certain amount of time the access switches if it was locked.
