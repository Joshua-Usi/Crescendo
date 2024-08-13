*sv_cheats 1*

My previous Cvar system involved a simple unordered map that stored string key-value pairs where the key was an identifier and the value was just a straight string of data. While this system worked, it was rather inefficient and I had to avoid reading from the system often due to the overhead of having to convert strings to integers and floats.

Hence a new solution was needed

Introducing CVar 2.0