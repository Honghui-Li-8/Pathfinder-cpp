#include <SFML/Graphics.hpp>
#include <windows.h>
#include <time.h>
#include <iostream>
#include <vector>
using namespace sf;

void updateBoard(RenderWindow& app, Texture t, Sprite s, std::vector<std::vector<int>>& map, int size, int w);
void clearUp(RenderWindow& app, Texture t, Sprite s, std::vector<std::vector<int>>& map
    , std::vector<std::vector<int>>& info, int size, int w);
int getInput(RenderWindow& app, Texture t, Sprite s, std::vector<std::vector<int>>& map
    , std::vector<std::vector<int>>& info, int size, int w);
void aStar(RenderWindow& app, Texture t, Sprite s, std::vector<std::vector<int>>& map
    , std::vector<std::vector<int>>& info, int size, int w);
int getEC(int x1, int y1, int x2, int y2);

// structures
struct node {
	int x = 0;
	int y = 0;
	int cc = 0; // current cost
	int ec = 0; // estimate cost
};
struct qnode {
	node current;
	int priority = 0;
	struct qnode* next;
};
class priority_Q {
public:
	qnode* root = NULL;


	void insert(node n) {
		qnode* temp;
		qnode* iterator;

		temp = new qnode;
		temp->current = n;
		temp->priority = n.cc + n.ec;

		if (root == NULL)
		{
			temp->next = root;
			root = temp;
		}
		else if (temp->priority < root->priority) {
			temp->next = root;
			root = temp;
		}
		else
		{
			iterator = root;
			// find the right place to append to
			while (iterator->next != NULL && iterator->next->priority <= temp->priority) {
				iterator = iterator->next;
			}
			// inserted node should go after iterator's position
			temp->next = iterator->next;
			iterator->next = temp;
		}
	}

	node pop() {
		// shoudln't call pop() when root is nullptr 
		node temp = root->current;
		root = root->next;
		return temp;
	}
};

// global variables
const int size = 50;
int start_x = 0;
int start_y = 0;
int end_x = 0;
int end_y = 0;



// won't need temp after
int temp = 0;

int main()
{
    srand(time(0));

    RenderWindow app(VideoMode(800, 800), "Test, is it working?");

    int w = 16;
    std::vector<std::vector<int>> map;
    // 0=> block, 1=> taget(end), 2=>fortiner
    // 3=> visited, 4 => unvisited, 5=> start 
    for (int i = 0;i< size;++i) {
        map.push_back(std::vector<int> {});
        for (int j = 0; j < size; ++j) {
            map[i].push_back(4);
        }
    }


    std::vector<std::vector<int>> info;
    // 0 = r, 1 = l, 2 = u, 3 = d
    // 99 for start, 100 for end
    // -1 is unvisited, -2 for block
    for (int i = 0; i < size; ++i) {
        info.push_back(std::vector<int> {});
        for (int j = 0; j < size; ++j) {
            info[i].push_back(-1);
        }
    }

    Texture t;
    t.loadFromFile("images/colorBlocks.jpg");
    Sprite s(t);
    // done initilization =========================

    // manully set up the board
    start_x = 1;
    start_y = 16;
    end_x = 45;
    end_y = 29;
    info[start_x][start_y] = 99;
    info[end_x][end_y] = 100;
    map[start_x][start_y] = 5;
    map[end_x][end_y] = 1;

    // setting blocks ===========
    /*
    for (int i = 5; i < 41; ++i) {
        info[6][i] = -2;
        map[6][i] = 0;
    }


    for (int i =  0; i < 50; ++i) {
        if (i == 36)
            continue;

        info[11][i] = -2;
        map[11][i] = 0;
    }


    for (int i = 0; i < 50; ++i) {
        if (i == 51)
            continue;

        info[19][i] = -2;
        map[19][i] = 0;
    }
    */
    //===========done seeting=============

    // call function aStar
    // aStar(app, t, s, map, info, size, w);

    // code to select cells
    while (app.isOpen())
    {
        int algoSelection = getInput(app, t, s, map, info, size, w);
        if (algoSelection == 0) {
            // A star
            aStar(app, t, s, map, info, size, w);
        }
        std::cout << "cycle finished\n";


        Event e;
        // hold until key pressed
        while (true) {
            app.pollEvent(e);
            if (e.type == Event::KeyPressed) {
                std::cout << "key is: " << e.key.code << std::endl;
                // 59 is backspace ( <--- )
                if (e.key.code == 59) {
                    // Delete pressed
                    std::cout << "Backspace pressed !!\n";
                    break;
                }
            }
        }
        
        // remove the search history
        clearUp(app, t, s, map, info, size, w);

    }

    return 0;
}

int getInput(RenderWindow& app, Texture t, Sprite s, std::vector<std::vector<int>>& map,
    std::vector<std::vector<int>>& info, int size, int w) {
    bool enter_flag = false;
    int algorithm = 0;
    int setStartEnd = 0;

    while (enter_flag != true) {
        // enter not been hit, keep getting input
        Vector2i pos = Mouse::getPosition(app);
        int width = app.getSize().x / size;
        int height = app.getSize().y / size;
        int x = pos.x / width;
        int y = pos.y / height;
        if (x >= 50) { x = 49; }
        if (y >= 50) { y = 49; }

        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();

            if (e.type == Event::MouseButtonPressed) {
                // 
                if (e.key.code == Mouse::Left) {
                    if (info[x][y] == -2) {
                        // remove block
                        info[x][y] = -1;
                        map[x][y] = 4;
                    }
                    else if (info[x][y] == -1) {
                        // add block
                        info[x][y] = -2;
                        map[x][y] = 0;

                    }
                }
                else if (e.key.code == Mouse::Right) {
                    if (setStartEnd == 0) {
                        if (x != end_x && y != end_y) {
                            // this place didn't set to end
                            // remove old start
                            info[start_x][start_y] = -1;
                            map[start_x][start_y] = 4;

                            // set start
                            start_x = x;
                            start_y = y;
                            info[x][y] = 99;
                            map[x][y] = 5;
                            setStartEnd = 1;
                        }
                    }
                    else if (setStartEnd == 1) {
                        if (x != start_x && y != start_y) {
                            // remove old end (traget)
                            info[end_x][end_y] = -1;
                            map[end_x][end_y] = 4;

                            // set end (target)
                            end_x = x;
                            end_y = y;
                            info[x][y] = 100;
                            map[x][y] = 1;
                            setStartEnd = 0;
                        }
                    }
                }
            }
            else if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Enter) {
                    // enter pressed
                    std::cout << "enter pressed !!\n";
                    enter_flag = true;
                }
            }
        }

        app.clear(Color::White);

        updateBoard(app, t, s, map, size, w);
    }

    return algorithm;
}

void clearUp(RenderWindow& app, Texture t, Sprite s, std::vector<std::vector<int>>& map, std::vector<std::vector<int>>& info, int size, int w) {
    // do nothing for now
    std::cout << "clear up!!\n";
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (map[i][j] == 3 || map[i][j] == 2 || map[i][j] == 5) {
                // if is visited cell or frontier or path (start)
                // reset to unvisited
                map[i][j] = 4;
                info[i][j] = -1;
            }

            if (i == start_x && j == start_y) {
                map[start_x][start_y] = 5;
                info[start_x][start_y] = 99;
            }
            else if (i == end_x && j == end_y) {
                map[end_x][end_y] = 1;
                info[end_x][end_y] = 100;
            }

        }
    }
    updateBoard(app, t, s, map, size, w);


}

void updateBoard(RenderWindow &app, Texture t, Sprite s, std::vector<std::vector<int>>& map, int size, int w) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
        {
            s.setTextureRect(IntRect(map[i][j] * w, 0, w, w));
            s.setPosition(i * w, j * w);
            app.draw(s);
        }

    app.display();
}

void aStar(RenderWindow& app, Texture t, Sprite s, std::vector<std::vector<int>>& map
    , std::vector<std::vector<int>>& info, int size, int w) {
    priority_Q frontier;
    int estimateCost = 0;
    estimateCost = getEC(start_x, start_y, end_x, end_y);
    frontier.insert(node{ start_x,start_y,0, estimateCost });

    node current;
    while (frontier.root != NULL) {
        current = frontier.pop();
        std::cout << "current.x: " << current.x << ", current.y" << current.y<< std::endl;

        if (current.x == end_x && current.y == end_y) {
            // found
            break;
        }

        if (info[current.x][current.y] == 100) {
            std::cout << "reach end\n";
            break;
        }
            

        // mark current as visited in map
        map[current.x][current.y] = 3;
        int cost = current.cc + 1;

        if (current.x > 0) {
            // have cell on left
            if (info[current.x - 1][current.y] == -1 || info[current.x - 1][current.y] == 100) {
                // unvisted
                estimateCost = getEC(current.x - 1, current.y, end_x, end_y);
                frontier.insert(node{ current.x - 1, current.y, cost, estimateCost });
                info[current.x - 1][current.y] = 0;
                // mark this cell as frontier
                map[current.x - 1][current.y] = 2;

            }
        }

        if (current.x < size - 1) {
            // have cell on right
            if (info[current.x + 1][current.y] == -1 || info[current.x + 1][current.y] == 100) {
                estimateCost = getEC(current.x + 1, current.y, end_x, end_y);
                frontier.insert(node{ current.x + 1, current.y, cost, estimateCost });
                info[current.x + 1][current.y] = 1;
                // mark this cell as frontier
                map[current.x + 1][current.y] = 2;
            }
        }

        if (current.y > 0) {
            // have cell on below (down)
            if (info[current.x][current.y - 1] == -1 || info[current.x][current.y - 1] == 100) {
                estimateCost = getEC(current.x, current.y - 1, end_x, end_y);
                frontier.insert(node{ current.x, current.y - 1, cost, estimateCost });
                info[current.x][current.y - 1] = 2;
                // mark this cell as frontier
                map[current.x][current.y - 1] = 2;
            }
        }

        if (current.y < size - 1) {
            // have cell on above (up)
            if (info[current.x][current.y + 1] == -1 || info[current.x][current.y + 1] == 100) {
                estimateCost = getEC(current.x, current.y + 1, end_x, end_y);
                frontier.insert(node{ current.x, current.y + 1, cost, estimateCost });
                info[current.x][current.y + 1] = 3;
                // mark this cell as frontier
                map[current.x][current.y + 1] = 2;
            }
        }

        // update the board
        // remark the start, been chaneged in first loop
        map[start_x][start_y] = 5;
        map[end_x][end_y] = 1;
        updateBoard(app, t, s, map, size, w);
    }

    if (frontier.root == NULL) {
        // can't reach the target
        std::cout << "no path to target\n";
        return;
    }

    std::cout << "reach here, should be founded\n";
    // display the path
    std::vector<int> path_x;
    std::vector<int> path_y;
    int current_x = end_x;
    int current_y = end_y;

    // remark the start
    info[start_x][start_y] = 99;
    while (info[current_x][current_y] != 99) {
        path_x.push_back(current_x);
        path_y.push_back(current_y);
        int temp = info[current_x][current_y];
        if (temp == 0)
            current_x += 1;
        else if (temp == 1)
            current_x -= 1;
        else if (temp == 2)
            current_y += 1;
        else if (temp == 3)
            current_y -= 1;
    }

    // path_x, path_y should have the path 
    int count = 1;
    for (int i = path_x.size() - 1; i > 0; --i) {
        map[path_x[i]][path_y[i]] = 5;
        ++count;
        updateBoard(app, t, s, map, size, w);
    }

    map[end_x][end_y] = 1;
    updateBoard(app, t, s, map, size, w);

}


int getEC(int x1, int y1, int x2, int y2) {
    int result = 0;

    if (x1 < x2)
        result += x2 - x1;
    else
        result += x1 - x2;


    if (y1 < y2)
        result += y2 - y1;
    else
        result += y1 - y2;

    return result;
}












