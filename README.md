# pianoSightReadingAssistant
Digital Design with microcontroller final project:
Link: https://people.ece.cornell.edu/land/courses/ece4760/FinalProjects/f2014/org5/website/website/index.html
Featured in: Circuit Cellar Magazine #308, pp 34-41, March 2016

With the advent of musical video games such as Guitar Hero and Rock Band, we thought it would be a great project to take the core idea of these games and make into something useful for the intermediate to advanced musician. Our design shows sheet music on a TV and compares the notes to keyboard input to see if the user is playing the music correctly. If the user presses the correct notes at the correct time, (s)he gains points. If the user plays incorrectly, (s)he loses points.
The interface consists solely of one microcontroller (ATMEGA 1284p) connected to a TV screen via NTSC. It is connected to the keyboard via serial connection to the other 1284p that we are using. The interface is responsible for displaying music, updating music, and calculating score based on keyboard input via serial connection.

