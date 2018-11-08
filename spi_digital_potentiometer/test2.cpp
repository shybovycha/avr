class Board {
    std::vector<PieceOnBoard> pieces;
};

class Piece {
    PieceColor color;
    PieceType type;
};

class PieceOnBoard : public Piece {
    Position position;
};

enum PieceColor {
    WHITE,
    BLACK
};

enum PieceType {
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING
};

class History {
    std::vector<HistoryEntry> entries;
};

class HistoryEntry {
    Move white, black;
};

class Move {
    Position from, to;
};

bool validateMove(std::shared_ptr<Board> board, std::shared_ptr<History> history, Move move) {
    Option<Piece> pieceOpt = board.pieces.filter(p -> p.position.equals(move.from));

    return pieceOpt.map(piece -> validatePieceMove(board, history, piece, move)).getOrElse(false);
}

bool validatePieceMove(std::shared_ptr<Board> board, std::shared_ptr<History> history, Piece piece, Move move) {
    switch (piece.type) {
        case PAWN:
            return validatePawnMove(board, history, piece, move.from, move.to);
        case ROOK:
            return validateRookMove(board, history, piece, move.from, move.to);
        case KNIGHT:
            return validateKnightMove(board, history, piece, move.from, move.to);
        case BISHOP:
            return validateBishopMove(board, history, piece, move.from, move.to);
        case QUEEN:
            return validateQueenMove(board, history, piece, move.from, move.to);
        case KING:
            return validateKingMove(board, history, piece, move.from, move.to);
        break;
    };
}

bool validatePawnMove(std::shared_ptr<Board> board, std::shared_ptr<History> history, Piece piece, Position from, Position to) {
    if (to.row)
}
