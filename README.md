# pianoSightReadingAssistant
Digital Design with microcontroller final project:
Link: https://people.ece.cornell.edu/land/courses/ece4760/FinalProjects/f2014/org5/website/website/index.html
Featured in: Circuit Cellar Magazine #308, pp 34-41, March 2016

With the advent of musical video games such as Guitar Hero and Rock Band, we thought it would be a great project to take the core idea of these games and make into something useful for the intermediate to advanced musician. Our design shows sheet music on a TV and compares the notes to keyboard input to see if the user is playing the music correctly. If the user presses the correct notes at the correct time, (s)he gains points. If the user plays incorrectly, (s)he loses points.
The interface consists solely of one microcontroller (ATMEGA 1284p) connected to a TV screen via NTSC. It is connected to the keyboard via serial connection to the other 1284p that we are using. The interface is responsible for displaying music, updating music, and calculating score based on keyboard input via serial connection.

The sheet music is displayed in the following fashion:

Note structures are created in order for the entire song.
Notes are placed sequentially until their total value is more than a full measure.
A measure (bar) line is placed and the beat is reset
Numbers 2 and 3 are repeated until the line is full.
A new line is started and steps 2 through 4 are repeated until the page is full (or the last note is drawn).
Staff lines and clefs are drawn to complete the page.
The display is halted until the user plays the entire page.
The interface was designed using a mapping of bit values - each kind of note was essentially drawn as a square of 0s and 1s - 1 being a white pixel and 0 being a black pixel. In this fashion, all notes and clefs were drawn.
