#ifndef LOGGER_  // Include guard to prevent multiple inclusions of this header file
#define LOGGER_

#include <ctime>      // For time-related functions
#include <fstream>    // For file stream operations
#include <iostream>   // For standard input/output stream
#include <sstream>    // For string stream operations
#include <string>     // For using the string class
#include <mutex>      // For mutexes to handle thread safety

using namespace std;

/**
 * @enum LogLevel
 * @brief Enumeration for log levels.
 */
enum LogLevel
{
    DEBUG,     ///< Debugging information
    INFO,      ///< Informational messages
    WARNING,   ///< Warning messages
    ERROR,     ///< Error messages
    CRITICAL   ///< Critical error messages
};

/**
 * @class Logger
 * @brief A singleton logger class for logging messages to a file.
 */
class Logger
{
public:
    /**
     * @brief Get the singleton instance of Logger.
     *
     * @param filename The name of the log file (default is "default.log").
     * @return Reference to the Logger instance.
     */
    static Logger& getInstance(const string& filename = "default.log")
    {
        static Logger instance(filename);  // Static instance of Logger
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Log a message with a specific log level.
     *
     * @param level The log level of the message.
     * @param message The message to log.
     */
    void log(const LogLevel level, const string& message)
    {
        std::lock_guard<std::mutex> lock(mutex_);  // Lock mutex for thread safety
        if (logFile.is_open())  // Check if the log file is open
        {
            logFile << formatLogEntry(level, message);  // Write formatted log entry to file
            logFile.flush();  // Flush the stream to ensure the message is written
        }
    }

    /**
     * @brief Overload the << operator to log string messages.
     *
     * @param message The string message to log.
     * @return Reference to the Logger instance.
     */
    Logger& operator<<(const string& message)
    {
        log(currentLevel, message);  // Log the message with the current log level
        return *this;
    }

    /**
     * @brief Overload the << operator to set the current log level.
     *
     * @param level The log level to set.
     * @return Reference to the Logger instance.
     */
    Logger& operator<<(const LogLevel level)
    {
        currentLevel = level;  // Set the current log level
        return *this;
    }

    /**
     * @brief Overload the << operator to log integer values.
     *
     * @param value The integer value to log.
     * @return Reference to the Logger instance.
     */
    Logger& operator<<(const int value)
    {
        log(currentLevel, to_string(value));  // Convert integer to string and log it
        return *this;
    }

    /**
     * @brief Overload the << operator to log C-style string messages.
     *
     * @param message The C-style string message to log.
     * @return Reference to the Logger instance.
     */
    Logger& operator<<(const char* message)
    {
        log(currentLevel, message);  // Log the C-style string message
        return *this;
    }

    /**
     * @brief Overload the << operator to handle manipulators (like std::endl).
     *
     * @param manip The manipulator to apply.
     * @return Reference to the Logger instance.
     */
    Logger& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
        {
            log(currentLevel, "\n");  // Log a newline
            logFile.flush();  // Flush the stream
            firstCall = true;  // Reset firstCall flag
        }
        return *this;
    }

    /**
     * @brief Overload the << operator to handle other iostream manipulators.
     *
     * @param manip The manipulator to apply.
     * @return Reference to the Logger instance.
     */
    Logger& operator<<(std::ios_base& (*manip)(std::ios_base&))
    {
        manip(std::cout);  // Apply manipulator to std::cout
        return *this;
    }

private:
    /**
     * @brief Private constructor to initialize the logger with a filename.
     *
     * @param filename The name of the log file to open.
     */
    explicit Logger(const string& filename)
        : currentLevel(INFO), firstCall(true)  // Initialize log level and first call flag
    {
        logFile.open(filename, ios::app);  // Open the log file in append mode
        if (!logFile.is_open())  // Check if the log file was opened successfully
        {
            cerr << "Error while opening protocol file." << endl;  // Print error message to stderr
        }
    }

    /**
     * @brief Destructor to close the log file.
     */
    ~Logger()
    {
        logFile.close();  // Close the log file upon destruction of the Logger instance
    }

    ofstream logFile;  ///< File stream for logging
    LogLevel currentLevel;  ///< Current log level
    bool firstCall;  ///< Flag to indicate if it's the first log call
    std::mutex mutex_;  ///< Mutex for thread safety

    /**
     * @brief Format the log entry with timestamp and log level.
     *
     * @param level The log level of the message.
     * @param message The message to log.
     * @return Formatted log entry as a string.
     */
    string formatLogEntry(const LogLevel level, const string& message)
    {
        const time_t now = time(nullptr);  // Get the current time
        const tm* timeinfo = localtime(&now);  // Convert to local time
        char timetxt[20];
        strftime(timetxt, sizeof(timetxt), "%Y-%m-%d %H:%M:%S", timeinfo);  // Format the time

        ostringstream logEntry;  // String stream to build the log entry
        if (firstCall)
        {
            logEntry << "[" << timetxt << "] " << levelToString(level) << ": " << message;  // First log entry format
            firstCall = false;  // Set firstCall to false after the first entry
        }
        else
        {
            logEntry << message;  // Subsequent log entries
        }
        logEntry.flush();  // Flush the string stream
        return logEntry.str();  // Return the formatted log entry as a string
    }

    /**
     * @brief Convert log level enum to string.
     *
     * @param level The log level to convert.
     * @return String representation of the log level.
     */
    static string levelToString(const LogLevel level)
    {
        switch (level)
        {
        case DEBUG: return "DEBUG";  ///< Return string representation of DEBUG
        case INFO: return "INFO";    ///< Return string representation of INFO
        case WARNING: return "WARNING";  ///< Return string representation of WARNING
        case ERROR: return "ERROR";  ///< Return string representation of ERROR
        case CRITICAL: return "CRITICAL";  ///< Return string representation of CRITICAL
        default: return "UNKNOWN";  ///< Return UNKNOWN for unrecognized log levels
        }
    }
};

#endif