To be completely frank, this part of the project is not done. I am incredibly 
behind in general, and this section should not have been terribly difficult to 
complete given that this is basically just a project from the Systems
Programming course. However, my code for that project was on WPI's Solar-10
Gitlab, which appears to no longer exist. With that in mind, I do not have time
to write the whole thing over again just to integrate the message passing from
the earlier section, simple as that last part may be.

My plan, at least, had been to create threads for ranges of rows, similar to
how threads were created for subranges of 1 to n in part 1. Each thread would
calculate the next state of each cell, then send a message containing that
information to the main thread, which would receive messages from all of the
children threads, and use those submatrices computer by each thread to create
the next cell matrix.