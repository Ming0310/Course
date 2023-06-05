# search.py
# ---------
# Licensing Information:  You are free to use or extend these projects for
# educational purposes provided that (1) you do not distribute or publish
# solutions, (2) you retain this notice, and (3) you provide clear
# attribution to UC Berkeley, including a link to http://ai.berkeley.edu.
# 
# Attribution Information: The Pacman AI projects were developed at UC Berkeley.
# The core projects and autograders were primarily created by John DeNero
# (denero@cs.berkeley.edu) and Dan Klein (klein@cs.berkeley.edu).
# Student side autograding was added by Brad Miller, Nick Hay, and
# Pieter Abbeel (pabbeel@cs.berkeley.edu).


"""
In search.py, you will implement generic search algorithms which are called by
Pacman agents (in searchAgents.py).
"""

import util

class SearchProblem:
    """
    This class outlines the structure of a search problem, but doesn't implement
    any of the methods (in object-oriented terminology: an abstract class).

    You do not need to change anything in this class, ever.
    """

    def getStartState(self):
        """
        Returns the start state for the search problem.
        """
        util.raiseNotDefined()

    def isGoalState(self, state):
        """
          state: Search state

        Returns True if and only if the state is a valid goal state.
        """
        util.raiseNotDefined()

    def getSuccessors(self, state):
        """
          state: Search state

        For a given state, this should return a list of triples, (successor,
        action, stepCost), where 'successor' is a successor to the current
        state, 'action' is the action required to get there, and 'stepCost' is
        the incremental cost of expanding to that successor.
        """
        util.raiseNotDefined()

    def getCostOfActions(self, actions):
        """
         actions: A list of actions to take

        This method returns the total cost of a particular sequence of actions.
        The sequence must be composed of legal moves.
        """
        util.raiseNotDefined()


def tinyMazeSearch(problem):
    """
    Returns a sequence of moves that solves tinyMaze.  For any other maze, the
    sequence of moves will be incorrect, so only use this for tinyMaze.
    """
    from game import Directions
    s = Directions.SOUTH
    w = Directions.WEST
    return  [s, s, w, s, w, w, s, w]

def depthFirstSearch(problem):
    """
    Search the deepest nodes in the search tree first.

    Your search algorithm needs to return a list of actions that reaches the
    goal. Make sure to implement a graph search algorithm.

    To get started, you might want to try some of these simple commands to
    understand the search problem that is being passed in:

    print("Start:", problem.getStartState())
    print("Is the start a goal?", problem.isGoalState(problem.getStartState()))
    print("Start's successors:", problem.getSuccessors(problem.getStartState()))
    """
    "*** YOUR CODE HERE ***"
    stack = util.Stack()
    visited_list={}
    path={}
    current_state=problem.getStartState()
    # initial
    for state in problem.getSuccessors(problem.getStartState()):
        stack.push(state)
    # search
    while stack.isEmpty() == False:
        current_state = stack.pop()
        visited_list[current_state[0]] = True
        if problem.isGoalState(current_state[0]):
            break
        candidate_state = problem.getSuccessors(current_state[0])
        for state in candidate_state:
            if state[0] not in visited_list.keys():
                stack.push(state)
                path[state]=current_state
    result = []
    result.append(current_state[1])
    while path.get(current_state):
        result.append(path[current_state][1])
        current_state = path[current_state]
    result.reverse()
    return result

def breadthFirstSearch(problem):
    """Search the shallowest nodes in the search tree first."""
    "*** YOUR CODE HERE ***"
    # fringe候选，walked走过的节点
    fringe = util.Queue()
    closed = []

    # 初始化，加入起点
    fringe.push((problem.getStartState(), []))

    while not fringe.isEmpty():
        cur_state, path = fringe.pop()

        # 到终点
        if problem.isGoalState(cur_state):
            return path

        if cur_state not in closed:
            # 后继节点
            next = problem.getSuccessors(cur_state)
            closed.append(cur_state)

            for state in next:
                if (state[0] not in closed):
                    #state[1]是str，用[]转化成list才能连接
                    fringe.push((state[0], path + [state[1]]))

def uniformCostSearch(problem):
    """Search the node of least total cost first."""
    "*** YOUR CODE HERE ***"
    priority_queue = util.PriorityQueue()
    priority_queue.push((problem.getStartState(),[]),0)
    visited = []
    while not priority_queue.isEmpty():
        current_state,action = priority_queue.pop()
        if problem.isGoalState(current_state):
            return action
        if current_state not in visited:
            next_state = problem.getSuccessors(current_state)
            for position,direction,cost in next_state:
                priority_queue.push((position,action + [direction]),problem.getCostOfActions(action + [direction]))
            visited.append(current_state)

def nullHeuristic(state, problem=None):
    """
    A heuristic function estimates the cost from the current state to the nearest
    goal in the provided SearchProblem.  This heuristic is trivial.
    """
    return 0

def aStarSearch(problem, heuristic=nullHeuristic):
    """Search the node that has the lowest combined cost and heuristic first."""
    "*** YOUR CODE HERE ***"
    queue = util.PriorityQueue()
    cost={}
    path={}
    result=[]
    flag = True
    # initial
    current_state=problem.getStartState()
    cost[current_state]=[0,heuristic(problem.getStartState(), problem)]
    queue.push(current_state,0)
    path[problem.getStartState()]=[(),'Stop']
    while queue.isEmpty() == False:
        current_state = queue.pop()
        if problem.isGoalState(current_state):
            break
        candidate_state = problem.getSuccessors(current_state)
        for state in candidate_state:
            if state[0] not in path.keys():
                cost[state[0]]=[cost[current_state][0] + state[2], heuristic(state[0], problem)]
                queue.push(state[0],cost[state[0]][0]+cost[state[0]][1])
                path[state[0]]=[current_state,state[1]]
            else:
                newcost = state[2]+cost[current_state][0]
                if newcost < cost[state[0]][0]: # 更改父节点的指针并更新代价cost
                    flag = False # 测试
                    cost[state[0]][0] = newcost
                    path[state[0]] = [current_state,state[1]]
                    queue.update(state[0],newcost+cost[state[0]][1])
    while path.get(current_state) and path[current_state][1] != 'Stop':
        result.append(path[current_state][1])
        current_state = path[current_state][0]
    result.reverse()
    return result



# Abbreviations
bfs = breadthFirstSearch
dfs = depthFirstSearch
astar = aStarSearch
ucs = uniformCostSearch
