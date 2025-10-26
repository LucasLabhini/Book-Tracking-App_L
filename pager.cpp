#include "pager.h"
#include <algorithm>
#include <fstream>

using json = nlohmann::json;

static const std::string LIBRARY_FILE = "books.json";
Book Book::fromJSON(const nlohmann::json& j) {
    Book b;
    b.id = j["id"];
    b.title = j["title"];
    b.author = j["author"];
    b.datePublished = j["datePublished"];
    b.genre = j["genre"];
    if (j.contains("pageProgress")) b.pageProgress = j["pageProgress"];
    if (j.contains("inLibrary")) b.inLibrary = j["inLibrary"];
    if (j.contains("section")) b.section = static_cast<Status>(j["section"]);
    return b;
}

nlohmann::json Book::toJSON() const {
    return nlohmann::json{
        {"id", id},
        {"title", title},
        {"author", author},
        {"datePublished", datePublished},
        {"genre", genre},
        {"pageProgress", pageProgress},
        {"inLibrary", inLibrary},
        {"section", static_cast<int>(section)}
    };
}
namespace Books {

// Helper: load library from JSON file
static std::vector<Book> loadLibrary() {
    std::ifstream file(LIBRARY_FILE);
    std::vector<Book> books;
    if (file.is_open()) {
        json j;
        file >> j;
        for (auto& item : j) {
            books.push_back(Book::fromJSON(item));
        }
    }
    return books;
}

// Helper: save library to JSON file
static void saveLibrary(const std::vector<Book>& books) {
    std::ofstream file(LIBRARY_FILE);
    if (file.is_open()) {
        json j = json::array();
        for (auto& b : books) {
            j.push_back(b.toJSON());
        }
        file << j.dump(4); // pretty-print
    }
}

// Find book by id
Book* findBook(int id, std::vector<Book>& books) {
    auto it = std::find_if(books.begin(), books.end(),
                           [id](const Book& b){ return b.id == id; });
    return it != books.end() ? &(*it) : nullptr;
}

bool addBook(const Book &b, Status section) {
    auto books = loadLibrary(); // load JSON

    for (auto &book : books) {
        if (book.id == b.id) {
            if (!book.inLibrary) {
                // Reactivate the book
                book.inLibrary = true;
                book.section = section;  // optional: update section
                saveLibrary(books);      // persist change
                return true;
            }
            return false; // already in library
        }
    }

    // Book not found in JSON -> do nothing
    return false;
}

bool removeBook(int id) {
    auto books = loadLibrary();  // Load the library from the file
    Book* b = findBook(id, books);
    if (!b || !b->inLibrary) return false;

    b->inLibrary = false;
    saveLibrary(books);           // Save the changes
    return true;
}

bool changeSection(int id, Status newSection) {
    auto books = loadLibrary();
    Book* b = findBook(id, books);
    if (!b) return false;

    b->section = newSection;
    saveLibrary(books);
    return true;
}

bool updateProgress(int id, int page) {
    auto books = loadLibrary();
    Book* b = findBook(id, books);
    if (!b || b->section != Status::Reading) return false;

    b->pageProgress = page;
    saveLibrary(books);
    return true;
}

std::vector<Book> getBooks(std::optional<Status> section) {
    auto books = loadLibrary();
    std::vector<Book> result;
    for (auto& b : books) {
        if (!section.has_value() || b.section == section.value())
            result.push_back(b);
    }
    return result;
}

std::vector<Book> getBooksSorted(std::optional<Status> section, SortBy sortBy) {
    auto books = getBooks(section);
    auto cmp = [&](const Book &a, const Book &b) {
        switch(sortBy) {
        case SortBy::Title:  return a.title < b.title;
        case SortBy::Author: return a.author < b.author;
        case SortBy::Date:   return a.datePublished < b.datePublished;
        case SortBy::Genre:  return a.genre < b.genre;
        }
        return false;
    };
    std::sort(books.begin(), books.end(), cmp);
    return books;
}

std::vector<Book> searchBooks(const std::string& term, std::optional<Status> section) {
    auto books = getBooks(section);
    std::vector<Book> result;

    std::string lowerTerm = term;
    std::transform(lowerTerm.begin(), lowerTerm.end(), lowerTerm.begin(), ::tolower);

    for (auto& b : books) {
        std::string lowerTitle = b.title;
        std::string lowerAuthor = b.author;
        std::string lowerGenre = b.genre;

        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        std::transform(lowerAuthor.begin(), lowerAuthor.end(), lowerAuthor.begin(), ::tolower);
        std::transform(lowerGenre.begin(), lowerGenre.end(), lowerGenre.begin(), ::tolower);

        if (lowerTitle.find(lowerTerm) != std::string::npos ||
            lowerAuthor.find(lowerTerm) != std::string::npos ||
            lowerGenre.find(lowerTerm) != std::string::npos)
        {
            result.push_back(b);
        }
    }

    return result;
}

} // namespace Books
