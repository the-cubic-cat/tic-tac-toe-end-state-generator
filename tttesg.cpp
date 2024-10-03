#include <iostream>
#include <limits>
#include <array>
#include <vector>
#include <random>
#include <functional>
#include <fstream>
#include <iostream>

using std::cout;
using std::cin;

int getIntInput(bool requirePositive = true)
{
    int r{};
    std::string error{};
    if (requirePositive)
        { error = "Positive integer input expected. Try again.\n"; }
    else
        { error = "Integer input expected. Try again.\n"; }
    
    do
    {
        if (cin.fail())
        {
            cout << error;
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        cin >> r;

        if (requirePositive && r <= 0)
            { cin.setstate(std::ios_base::failbit); }

    } while (cin.fail());

    return r;
}
bool getBoolInput()
{
    char input{};
    bool repeat{false};
    do
    {
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cin.get(input);
        input = std::toupper(input);
        repeat = false;
        
        if (input != 'Y' && input != 'N')
        {
            repeat = true;
            cout << "Y/N expected. Try again.\n";
        }
        
    } while (repeat);

    if (input == 'Y') { return true; }
    else              { return false; }
}

int randomInt(int upperBound)
{
    return rand() % upperBound;
}

enum class Cell
{
    empty,
    X,
    O,
    unknown
};
std::ostream& operator<<(std::ostream& out, Cell cell)
{
    // depending on the cell, print corresponding character
    std::string printWhat{};

    switch (cell)
    {
    case Cell::empty:
        printWhat = "- ";
        break;
    case Cell::X:
        printWhat = "X ";
        break;
    case Cell::O:
        printWhat = "O ";
        break;
    default:
        printWhat = "? ";
    }

    out << printWhat;
    return out;
}
// inverts O into X and X into O. empty stays empty
void invertCell(Cell& outCell)
{
    switch(outCell)
    {
    case Cell::X:
        outCell = Cell::O;
        break;
    case Cell::O:
        outCell = Cell::X;
        break;
    default:
        break;
    }
}

struct Coords
{
    int x;
    int y;

    Coords operator=(const Coords& c)
    {
        // self-assignment check
	    if (this == &c) { return *this; }

        x = c.x;
        y = c.y;

        return *this;
    }

    friend Coords operator+(const Coords& coords1, const Coords& coords2)
    {
        return {coords1.x + coords2.x, coords1.y + coords2.y};
    }
    Coords operator+=(const Coords& toAdd)
    {
        (*this) = (*this) + toAdd;
        return (*this);
    }

    friend Coords operator-(const Coords& coords1, const Coords& coords2)
    {
        return {coords1.x - coords2.x, coords1.y - coords2.y};
    }
    Coords operator-=(const Coords& toSubtract)
    {
        (*this) = (*this) - toSubtract;
        return (*this);
    }
    Coords operator-() const
    {
        return Coords{0, 0} - (*this);
    }

    operator std::string() const
    {
        return std::to_string(x) + "; " + std::to_string(y);
    }
};

class Board
{
public:
    Cell whoWon{Cell::unknown};

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    // make a somewhat miserable 2d vector
    Board(int width, int height)
        : m_board{}
        , m_width{width}
        , m_height{height}
    {
        // creates a column with empty-initialized cells
        std::vector<Cell> column(height);

        // fills the aray with copies of that column
        for (int i{0}; i < width; i++)
        {
            m_board.push_back(column);
        }
    }
    
    // copy, move, destroy are all default, since vector is a move-capable type
    ~Board() = default;
    Board(const Board& b) = default;
    Board& operator=(const Board& a) = default;

    Board(Board&& b) noexcept = default;
    Board& operator=(Board&& a) noexcept = default;
    
    // couldn't get multidimensional subscript to work
    Cell& operator() (int x, int y) { return m_board[x][y]; }
    Cell& operator() (Coords coords) { return m_board[coords.x][coords.y]; }

    // these are used for read-only access, such as on consts
    Cell getCell(int x, int y) const { return m_board[x][y]; }
    Cell getCell(Coords coords) const { return m_board[coords.x][coords.y]; }

    // comparison
    friend bool operator==(const Board& b1, const Board& b2)
    { return b1.m_board == b2.m_board; }

    // checks whether the specified coords represent a valid position on board
    bool coordsInRange(Coords coords) const
    {
        if (coords.x < getWidth()  && coords.x >= 0 
         && coords.y < getHeight() && coords.y >= 0)
        {
            return true;
        }
        return false;
    }

    // outputs the current state of the board to designated ostream
    // (cout by default)
    void printBoard(std::ostream& output = cout)
    {
        // a printcel must be a kind of soyjack who really loves printers
        auto printCell = [this, &output](int x, int y)
        {
            // add newline before character when needed
            if (x == 0) { output << "\n"; }
            // print the character
            output << (*this)(x, y);
        };

        forEachCell(printCell);

        // display board state at the bottom
        std::string boardState{};

        switch (whoWon)
        {
        case Cell::empty:
            boardState = "draw";
            break;
        case Cell::X:
            boardState = "X win";
            break;
        case Cell::O:
            boardState = "O win";
            break;
        default:
            boardState = "UNKNOWN STATE! THERE IS PROBABLY A BUG IN THE CODE!";
        }

        output << "\n^\n" << boardState << "\n";
        output<< "Number of empty cells: " << getEmptyCells().size() << "\n";
    }
    
    // returns a vector of coordinates of all empty cells
    std::vector<Coords> getEmptyCells()
    {
        // vector of currently found empty cells
        std::vector<Coords> r{};

        // make lambda
        auto storeEmptyToVector = [this, &r](int x, int y)
        {
            // check if current cell is empty
            if ((*this)(x, y) == Cell::empty)
            {
                //if it is, add to vector
                Coords coords{x, y};
                r.push_back(coords);
            }
        };
        // run lambda for each cell (i am genuinely proud of this)
        forEachCell(storeEmptyToVector);
        // return final list
        return r;
    }
private:
    // run some function for each cell on board
    void forEachCell(auto& doWhat)
    {
        for (int y{0}; y < getHeight(); y++)
        {
            for (int x{0}; x < getWidth(); x++)
            {
                doWhat(x, y);
            }
        }
    }
    // implemented with vectors because this program is not performance-critical
    std::vector<std::vector<Cell>> m_board;
    int m_width;
    int m_height;
};

// the return value is whether it is possible to make a turn or not;
// if false, the game is over with a draw, all cells are filled
bool makeTurn(Board& board, Cell whosTurn, Coords& outCoords)
{
    // get list of all empty cells
    auto emptyCells = board.getEmptyCells();
    // if no empty cells, game is over
    if (emptyCells.size() == 0)
    {
        return false;
    }
    // select random cell
    int randomIndex{randomInt(emptyCells.size())};
    Coords selectedCellCoords = emptyCells[randomIndex];
    // and set it to the corresponding value
    board(selectedCellCoords) = whosTurn;

    // let the caller know what coords we chose
    outCoords = selectedCellCoords;

    return true;
}

// checks if a line has been formed; if it has, returns true, otherwise false
bool gameEndCheck(const Board& board, Coords turnCoords, int lineLength)
{
    const Cell whosTurn = board.getCell(turnCoords);

    // lambda to count in a direction
    auto countLineLengthInDirection = 
        [&board, &turnCoords, &whosTurn] (Coords direction)
    {
        int r{0};

        // stores where we are
        Coords currentCellCoords{turnCoords};

        // while we are inside board and the cell is in correct state
        while (board.coordsInRange(currentCellCoords)
            && board.getCell(currentCellCoords) == whosTurn)
        {
            // add one to the counter of line length
            r++;
            // and move in specified direction
            currentCellCoords += direction;
        }
        return r;
    };

    bool gameEnded = false;
    
    std::array<Coords, 4> directions
    {{
        {1, 0}, // horizontal
        {0, 1}, // vertical
        {1, 1}, // major diagonal (\)
        {1, -1} // minor diagonal (/)
    }};

    // for each one of the directions
    for (Coords direction : directions)
    {
        int currentLineLength{0};
        // count line in that direction, then in opposite one, then subtract 1
        // to compensate for overlap
        currentLineLength = countLineLengthInDirection(direction)
            + countLineLengthInDirection(-direction) - 1;
        // if the length we got is longer than the game's line length
        // one of the players won!
        if (currentLineLength >= lineLength)
        {
            gameEnded = true;
            break;
        }
    }

    return gameEnded;
}

const Board simulateGame(int boardWidth, int boardHeight, int lineLength)
{
    // create board with specified size
    Board b{boardWidth, boardHeight};

    // X always goes first
    Cell whosCurrentTurn{Cell::X};
    // here we store the coords of the last turn
    Coords lastTurnCoords;

    // by default we say that no one won (draw), might override later
    b.whoWon = Cell::empty;

    // while we can make a turn, make a turn, and then
    while (makeTurn(b, whosCurrentTurn, lastTurnCoords))
    {
        // if a line has been detected
        if (gameEndCheck(b, lastTurnCoords, lineLength))
        {
            // we know who won, and then end game
            b.whoWon = whosCurrentTurn;
            break;
        }
        // else we give the turn to the other player
        invertCell(whosCurrentTurn);
    }

    // return board
    return b;
}

int countNumberOfEndBoards(int width, int height, int length, int quitThreshold
    , bool printResults, std::vector<Board>& boards)
{
    // this is the number of times the program has looped without adding new
    // boards to the vector (consecutively)
    int uselessRepeats{0};

    while (uselessRepeats < quitThreshold)
    {
        Board newBoard{simulateGame(width, height, length)};

        bool addNewBoard{true};

        for (Board b : boards)
        {
            if (newBoard == b)
            {
                addNewBoard = false;
                ++uselessRepeats;
                break;
            }
        }

        if (addNewBoard)
        {
            boards.push_back(newBoard);
            if (printResults) 
            {
                newBoard.printBoard();
                cout << "Board #" << boards.size() << "\n";
            }
            uselessRepeats = 0;
        }
    }

    return boards.size();
}

void printBoardsToFile(const std::vector<Board>& seenBoards, int lineLength
    , int quitThreshold)
{
    std::string fileName = std::to_string(seenBoards[0].getHeight()) + "x"
        + std::to_string(seenBoards[0].getWidth()) + "_l" 
        + std::to_string(lineLength) + "_boards_log";

    std::ofstream file{"logs/" + fileName};
    
    file << "The number of boards counted before the quit threshold ("
        << quitThreshold << ") was exceeded is: "
        << seenBoards.size() << "\n";
    
    for (Board b : seenBoards)
    {
        b.printBoard(file);
    }
}

int main()
{
    srand(time(nullptr)); // resets rng

    int width;
    cout << "Input the width of the game board.\n";
    width = getIntInput();

    int height;
    cout << "Input the height of the game board.\n";
    height = getIntInput();

    int length;
    cout << "Input the line length.\n";
    length = getIntInput();

    int quitThreshold;
    cout << "Input the quit threshold (how many times the program must loop"
            " without result before it exits).\n";
    quitThreshold = getIntInput();

    bool printResults;
    cout << "Print boards as the program executes? (Y/N)\n";
    printResults = getBoolInput();

    cout << "Calculating...\n";

    std::vector<Board> seenBoards{};
    int numberOfEndBoards = countNumberOfEndBoards(width, height, length
        , quitThreshold, printResults, seenBoards);

    cout << "The number of boards counted before the quit threshold ("
        << quitThreshold << ") was exceeded is: "
        << numberOfEndBoards << "\n";
    
    cout << "Print results and all boards (" << numberOfEndBoards 
        << ") to file? (Y/N)\n";
    if (getBoolInput())
        { printBoardsToFile(seenBoards, length, quitThreshold); }

    return 0;
}