#include "space_explorer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// struct to store a planet to be visited and its estimated distance from the
// mixer
struct planetToVisit {
  unsigned int id;
  double estimatedDistFromMixer;
  // sum of all distances encountered
  double sum;
  int numDistances;
  int age;
};

// the list of planet that have been visited
struct visitedList {
  unsigned int numElems;
  unsigned int visited[500];
};

// ship state structure
struct ship_state {
  struct visitedList vl;
  struct Stack *toVisit;
};

// A structure to represent a stack
struct Stack {
  int top;
  unsigned int capacity;
  struct planetToVisit *array;
};

// stack modified from
// https://www.geeksforgeeks.org/implement-stack-using-array/ function to create
// a stack of given capacity. It initializes size of stack as 0
struct Stack *createStack(unsigned capacity) {
  struct Stack *stack = (struct Stack *)malloc(sizeof(struct Stack));
  stack->capacity = capacity;
  stack->top = -1;
  stack->array = (struct planetToVisit *)malloc(stack->capacity *
                                                sizeof(struct planetToVisit));
  return stack;
}

// Stack is full when top is equal to the last index
int isFull(struct Stack *stack) { return stack->top == stack->capacity - 1; }

// Stack is empty when top is equal to -1
int isEmpty(struct Stack *stack) { return stack->top == -1; }

// Function to add an item to stack.  It increases top by 1
void push(struct Stack *stack, struct planetToVisit item) {
  if (isFull(stack))
    return;
  stack->array[++stack->top] = item;
}

// Function to remove an item from stack.  It decreases top by 1
struct planetToVisit pop(struct Stack *stack) {
  if (isEmpty(stack)) {
    struct planetToVisit planet = {RAND_PLANET};
    return planet;
  }
  return stack->array[stack->top--];
}

// initialise the ship state structure
void initShipState(struct ship_state *ss) {
  // init the visted list
  ss->vl.numElems = 0;

  // init toVisit stack
  ss->toVisit = createStack(500);
}

// check whether a planet has been visited before
bool toVisitContains(struct ship_state *ss, unsigned int id) {
  bool contains = false;

  for (int i = 0; i < ss->toVisit->top + 1; i++) {
    if (ss->toVisit->array[i].id == id) {
      contains = true;
    }
  }

  return contains;
}

// returns a pointer to a planet in the toVist list
struct planetToVisit *getPlanetFromToVisit(struct ship_state *ss,
                                           unsigned int id) {
  for (int i = 0; i < ss->toVisit->top + 1; i++) {
    if (ss->toVisit->array[i].id == id) {
      return &ss->toVisit->array[i];
    }
  }
  return NULL;
}

// increment the ages of planet so that planets can be sorted by age
void updateAges(struct ship_state *ss) {
  for (int i = 0; i < ss->toVisit->top + 1; i++) {
    ss->toVisit->array[i].age++;
  }
}

// check whether a planet has been visited before
bool visitedContains(struct ship_state *ss, unsigned int id) {
  bool contains = false;

  for (int i = 0; i < ss->vl.numElems; i++) {
    if (ss->vl.visited[i] == id) {
      contains = true;
    }
  }

  return contains;
}

// add a new planet to the visited array
void addToVisited(struct ship_state *ss, unsigned int id) {
  ss->vl.visited[ss->vl.numElems] = id;
  ss->vl.numElems++;
}

// comparison function to pass to qsort
int comp(const void *elem1, const void *elem2) {
  struct planetToVisit p1 = *((struct planetToVisit *)elem1);
  struct planetToVisit p2 = *((struct planetToVisit *)elem2);
  if (p1.estimatedDistFromMixer < p2.estimatedDistFromMixer)
    return 1;
  if (p1.estimatedDistFromMixer > p2.estimatedDistFromMixer)
    return -1;
  if (p1.age > p2.age)
    return 1;
  if (p1.age < p2.age)
    return -1;
  if (p1.numDistances > p2.numDistances)
    return 1;
  if (p1.numDistances < p2.numDistances)
    return -1;
  return 0;
}

// sort the toVisit array
void sortToVisit(struct ship_state *ss) {
  qsort(ss->toVisit->array, ss->toVisit->top + 1, sizeof(struct planetToVisit),
        comp);
}

ShipAction space_hop(unsigned int crt_planet, unsigned int *connections,
                     int num_connections, double distance_from_mixer,
                     void *ship_state) {
  ShipAction sa;

  // create ptr to the ship state and init it if it hasn't been
  bool first = false;
  struct ship_state *ss;
  if (ship_state == 0x0) {
    ss = malloc(sizeof(struct ship_state));
    initShipState(ss);
    first = true;
  } else {
    ss = ship_state;
  }

  // add planet to visited list
  addToVisited(ss, crt_planet);

  // update planet ages
  updateAges(ss);

  // add unvisited connections to toVisit
  for (int i = 0; i < num_connections; i++) {
    if (!visitedContains(ss, connections[i])) {
      if (!toVisitContains(ss, connections[i])) {
        // add a new planet to toVisit
        struct planetToVisit p;
        p.id = connections[i];
        p.estimatedDistFromMixer = distance_from_mixer;
        p.sum = distance_from_mixer;
        p.numDistances = 1;
        p.age = 0;
        push(ss->toVisit, p);
      } else {
        // refine our estimate for the distance from the mixer for this
        // previously added planet by averaging all the distances we were at
        // when we have seen this planet in the connections
        struct planetToVisit *p = getPlanetFromToVisit(ss, connections[i]);

        p->sum += distance_from_mixer;
        p->numDistances++;

        p->estimatedDistFromMixer = p->sum / p->numDistances;
      }
    }
  }

  // sort ToVist so that we will be popping the planets that we can expect to be
  // closest to the mixer first
  sortToVisit(ss);

  // if this is our first move we jump to planet 0.
  // after testing, this reduced the average hops from ~21 to ~16. not all
  // seeds benefit from this, in some cases doing this simply adds 1 hop to the
  // journey but some seeds make big gains with this so it becomes worth it. i
  // came up with this because i noticed that on seeds that result in unusually
  // high hops, if i print out the planets they visit, 0 always appeared so i
  // can just cut off a big chunk of their journey by starting at 0.
  if (first) {
    sa.next_planet = 0;
  }
  // jump randomly if we are too far from the mixer
  else if (distance_from_mixer > 4) {
    sa.next_planet = RAND_PLANET;
  }
  // choose the next planet
  else {
    struct planetToVisit p;
    do {
      p = pop(ss->toVisit);
      sa.next_planet = p.id;
    } while (visitedContains(ss, sa.next_planet));
  }

  sa.ship_state = ss;
  return sa;
}
