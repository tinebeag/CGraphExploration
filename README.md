# C Graph Exploration Program
This is a program developed to solve a challenge given to me during my studies in which a random graph is generated (in the form of planets and connections) which must he explored to find the goal planet in the minimum number of moves.
The project is in two parts, first `space_explorer.c` and `space_explorer.h` are responsible for generating the graph and exposing the function `space_hop`. Then in `space_solution.c` in this body of the `space_hop` function, the solution must be developed. 
# Running the Program
Compile the C files and ensure to link in the math library. Using GCC this looks like:
`gcc space_explorer.c space_solution.c -o space_explorer -lm`
When executing the binary a seed should be provided as an argument to control the random generation like `./space_explorer 123`. Try various seeds to generate different graphs. 
