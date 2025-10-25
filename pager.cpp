#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

enum class Status { Reading, Read, Wish };

struct Book {
    int id;
    std::string title;
    std::string author;
    std::string datePublished;
    std::string genre;
    int pageProgress = 0;
    
    bool inLibrary = true;       // Always true if the user owns/interacted with it
    Status section;              // Reading, Read, or Wish

    // Create a Book from JSON
    static Book fromJSON(const nlohmann::json& j) {
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

    // Convert Book to JSON
    json toJSON() const {
        return json{
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
};

// Master list containing all books
static std::vector<Book> libraryList;

namespace Books {

    // ---- Utility ----
    Book* findBook(int id) {
        auto it = std::find_if(libraryList.begin(), libraryList.end(),
                               [id](const Book& b){ return b.id == id; });
        return it != libraryList.end() ? &(*it) : nullptr;
    }

    // ---- Core Functions ----
    bool addBook(const Book& book, Status section) {
        if (findBook(book.id)) return false; // Already exists

        Book copy = book;
        copy.inLibrary = true;
        copy.section = section;
        libraryList.push_back(copy);
        return true;
    }

    bool removeBook(int id) {
        auto it = std::remove_if(libraryList.begin(), libraryList.end(),
                                 [id](const Book& b){ return b.id == id; });
        if (it == libraryList.end()) return false;

        libraryList.erase(it, libraryList.end());
        return true;
    }

    bool changeSection(int id, Status newSection) {
        Book* b = findBook(id);
        if (!b) return false;

        b->section = newSection;
        return true;
    }

    bool updateProgress(int id, int page) {
        Book* b = findBook(id);
        if (!b || b->section != Status::Reading) return false;

        b->pageProgress = page;
        return true;
    }

    // ---- Retrieval ----
    std::vector<Book> getBooks(std::optional<Status> section = std::nullopt) {
        std::vector<Book> result;
        for (auto& b : libraryList) {
            if (!section.has_value() || b.section == section.value())
                result.push_back(b);
        }
        return result;
    }

    // ---- Sorting ----
    enum class SortBy { Title, Author, Date, Genre };

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

    // ---- Search ----
    std::vector<Book> searchBooks(const std::string& term, std::optional<Status> section = std::nullopt) {
        std::vector<Book> result;
        std::string lowerTerm = term;
        std::transform(lowerTerm.begin(), lowerTerm.end(), lowerTerm.begin(), ::tolower);

        for (auto& b : getBooks(section)) {
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
.
} // namespace Books
