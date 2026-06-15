from dataclasses import dataclass
from enum import Enum

@dataclass
class Position:
    x: int
    y: int

class Color(Enum):
    WHITE = "white"
    BLACK = "black"

class Rank(Enum):
    PAWN = "pawn"
    ROOK = "rook"
    KNIGHT = "knight"
    BISHOP = "bishop"
    QUEEN = "queen"
    KING = "king"

class Piece:
    def __init__(self, color: Color, rank: Rank):
        self.color: Color = Color(color)
        self.rank: Rank = Rank(rank)
    
    @property
    def letter(self):
        result = {
            Rank.PAWN: "P",
            Rank.ROOK: "R",
            Rank.KNIGHT: "N",
            Rank.BISHOP: "B",
            Rank.QUEEN: "Q",
            Rank.KING: "K",
        }[self.rank]
        return {
            Color.WHITE: result,
            Color.BLACK: result.lower()
        }[self.color]

SETUP = [
    Rank.ROOK, Rank.KNIGHT,
    Rank.BISHOP, Rank.QUEEN,
    Rank.KING, Rank.BISHOP,
    Rank.KNIGHT, Rank.ROOK,
]

class Move:
    def __init__(self, start: tuple[int], end: tuple[int]):
        self.start: Position = Position(*start)
        self.end: Position = Position(*end)

class Chess:
    def __init__(self):
        self.board: list[list[Piece]] = [
            [Piece(Color.WHITE, rank) for rank in SETUP],
            [Piece(Color.WHITE, Rank.PAWN) for _ in range(8)],
            *[[None for _ in range(8)] for _ in range(4)],
            [Piece(Color.BLACK, Rank.PAWN) for _ in range(8)],
            [Piece(Color.BLACK, rank) for rank in SETUP]
        ]

    def draw(self):
        picture = "\n".join([
            " ".join([
                piece.letter
                if piece else "."
                for piece in line
            ])
            for line in reversed(self.board)
        ])
        print(picture)
    
    def do_move(self, move: Move):
        self.board[move.end.y][move.end.x] = self.board[move.start.y][move.start.x]
        self.board[move.start.y][move.start.x] = None

    @property
    def possible_moves(self) -> list[Move]:
        result = []
        def add_move(*m):
            move = Move(*m)
            if len(self.board) <= move.end.y:
                return
            if len(self.board[0]) <= move.end.x:
                return
            if self.board[move.end.y][move.end.x] is not None:
                return
            result.append(move)
        for y, line in enumerate(self.board):
            for x, piece in enumerate(line):
                if piece:
                    if piece.rank == Rank.KING:
                        
                        for ax in [-1, 0, 1]:
                            for ay in [-1, 0, 1]:
                                if (ax, ay) != (0, 0):
                                    add_move((x, y), (x + ax, y + ay))
                        
                    elif piece.rank == Rank.PAWN:
                        pass
                    elif piece.rank == Rank.KNIGHT:
                        pass
                    elif piece.rank == Rank.ROOK:
                        pass
                    elif piece.rank == Rank.BISHOP:
                        pass
                    elif piece.rank == Rank.QUEEN:
                        pass
        return result


if __name__ == "__main__":
    game = Chess()
    game.do_move(Move((4, 1), (4, 3)))
    for move in game.possible_moves:
        print(f"({move.start.x}, {move.start.x}) ({move.end.x}, {move.end.y})")
    
    print("---------------")
    game.draw()

