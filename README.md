# Nonogram Terminal

A terminal-based Nonogram (also known as Picross or Paint by Numbers) puzzle game with vim-like controls.

Learn more about Nonograms: [Wikipedia - Nonogram](https://en.wikipedia.org/wiki/Nonogram)

## Description

Nonogram Terminal is a text-based implementation of the popular Nonogram puzzle game. Nonograms are picture logic puzzles where cells in a grid must be colored or left blank according to numbers at the side of the grid to reveal a hidden picture.

This project features:
- Terminal-based UI using ncurses
- Vim-inspired keyboard controls
- Multiple puzzle sets
- Save/load functionality
- Undo/redo system
- Visual selection mode for efficient solving

## Installation

### Prerequisites

- GCC or compatible C compiler
- ncurses library
- make

> [!NOTE]
> This project is designed for Unix-like systems (Linux, macOS).
> Terminal compatibility may vary and might not work on Windows.

### Building from Source

1. Clone the repository:
   ```
   git clone git@github.com:divingbeetle/Nonogram-Terminal.git
   cd Nonogram-Terminal
   ```

2. Compile the project using `make`:
   ```
   make
   ```

3. Run the game:
   ```
   ./bin/main
   ```

## How to Play

### Main Menu

Press any key to open the main menu. 
Use j/k or arrow keys to navigate and Enter to select an menu option.

Options:
- **New Game**: Start a new Nonogram puzzle
- **Continue**: Load the last saved game
- **Exit**: Quit the game


The following options are not yet implemented:
- **Settings**: Configure game settings (key bindings, colors, etc.)
- **HOW TO PLAY**: Interactive tutorial on Nonogram rules and game controls

### Basic Controls

The game uses vim-inspired keyboard controls:

#### Movement

- `h`, `j`, `k`, `l` or arrow keys: Move cursor left, down, up, right
- `0` or `I`: Move to the leftmost cell of the current row
- `$` or `A`: Move to the rightmost cell of the current row
- `g`: Move to the top row
- `G`: Move to the bottom row
- `w`, `W`: Jump 5 cells right
- `b`, `B`: Jump 5 cells left
- `o`: Jump 5 cells down
- `O`: Jump 5 cells up

#### Cell Marking

Filling a cell means marking the cell as part of the solution.
X-marking a cell means marking it as empty.

Temporary markings are filled/x-marked cells with different visual indicators, useful for testing or marking potential solutions.

- `f`: Fill a cell
- `x`: X-mark a cell
- `F`: Temporarily fill a cell
- `X`: Temporarily X-mark a cell
- `d`: Clear a cell
- `~`: Switch case (toggle between temporary and non-temporary markings)

#### Visual Mode

Visual mode allows you to select multiple cells for bulk operations.
You can use movement keys to expand the selection and cell marking keys to fill or x-mark all selected cells.
Press any other key to exit visual mode.

- `v`: Enter visual mode (select multiple cells)
- `V`: Enter visual mode and select entire row

#### Others

- `u`: Undo
- `U`: Redo
- `:`: Open command menu
- `q`: Quit the current game

#### Command Menu (`:`)

Command Menu allows you to perform various actions:
- **Auto XMark:** Automatically X-mark cells on completed rows/columns
- **Delete Temp Marks:** Remove all temporary markings from the board
- **Clear:** Clear the entire board
- **Capture:** Save the current board state
- **Restore Capture:** Restore the previously captured state
- **Save:** Save the game to disk
- **Quit:** Exit the game

## Save System

The game uses a file-based save system. 

Your progress is saved to `./save.dat` in the game directory. You can continue your last saved game by selecting "Continue" from the main menu.

## Puzzle Format

Puzzles are stored in JSON format in the `./puzzles` directory. 
Each JSON file represents a puzzle set containing multiple puzzles.

You can create your custom puzzle sets by following the provided JSON structure below.

```json
{
    "format_version": "0.2.0",
    "title": "Your Puzzle Set Title",
    "description": "Description of your puzzle set",
    "num_puzzles": 2,
    "puzzles": [
        {
            "id": 0,
            "title": "Puzzle Title",
            "author": "Your Name",
            "difficulty": 1,
            "rows": 5,
            "cols": 5,
            "row_clues": [
                [1, 1],
                [5],
                [5],
                [3],
                [1]
            ],
            "col_clues": [
                [2],
                [4],
                [4],
                [4],
                [2]
            ]
        },
        {
            "id": 1,
            "title": "Another Puzzle",
            "author": "Your Name",
            "difficulty": 2,
            "rows": 10,
            "cols": 10,
            "row_clues": [
                [7],
                [1, 2, 3],
                [1, 2, 1, 2],
                [1, 4, 2],
                [1, 1],
                [1, 1],
                [1, 6, 1],
                [1, 1, 1, 1],
                [1, 1, 1, 1],
                [10]
            ],
            "col_clues": [
                [10],
                [1, 1],
                [4, 4],
                [4, 1, 1],
                [1, 1, 1, 1],
                [4, 1, 1],
                [2, 1, 1],
                [2, 4],
                [2, 1],
                [7]
            ]
        }
    ]
}
```

### Puzzle File Guidelines:

1. Save your file with a `.json` extension in the `./puzzles` directory
2. Each puzzle set can contain multiple puzzles (up to 10)
3. For each puzzle:
   - `id`: Unique identifier within the set (starting from 0)
   - `title` and `author`: Text fields to identify the puzzle
   - `difficulty`: Integer value (0-5 recommended)
   - `rows` and `cols`: Dimensions of the puzzle grid
   - `row_clues`: Array of clues for each row (from top to bottom)
   - `col_clues`: Array of clues for each column (from left to right)

4. Clue format:
   - Each clue is an array of integers representing consecutive filled blocks
   - For example, `[1, 2, 3]` means "one filled cell, then a gap, then two filled cells, then a gap, then three filled cells"
   - Use `[0]` for a row or column with no filled cells

5. Create puzzles with dimensions that are multiples of 5

## Development Status

> [!NOTE]
> This project is currently inactive and is not being actively maintained.

This project started as a learning exercise to practice C programming.
While the core game functionality works, development has been paused indefinitely.

### Future Development

Resuming development would likely involve 
- Complete rewrite for code organization and maintainability
- Puzle format improvements and support for web-based puzzle databases
- Error handling and Memory management improvements
- Enhanced UI/UX with better ncurses usage

### Contributing

While active development is paused, bug reports and suggestions are welcome. 
However, please note that response times may be slow and major feature requests would likely require the aforementioned rewrite.

## Special Thanks

Special thanks to 소요선(soyo) from [LogicHome.org](https://logichome.org/) for graciously allowing the use of their high-quality puzzles as example content in this project.
