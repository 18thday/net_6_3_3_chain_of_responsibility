#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>


enum class LogMessageType {
    Warning,
    Error,
    FatalError,
    UnknownMessage
};

class LogMessage {
public:
    explicit LogMessage(LogMessageType type, std::string message)
    : type_(type), message_(std::move(message)) {
    }

    LogMessageType type() const {
        return type_;
    }
    const std::string& message() const {
        return message_;
    }

private:
    LogMessageType type_;
    std::string message_;
};

class LogMessageHandler {
public:
    virtual void setNextHandler(LogMessageHandler* next_handler) = 0;
    virtual void handle(const LogMessage& log) = 0;
};

class FatalErrorHandler : public LogMessageHandler {
public:
    void setNextHandler(LogMessageHandler* next_handler) override {
        next_handler_ = next_handler;
    }
    void handle(const LogMessage& log) override {
        if (log.type() == LogMessageType::FatalError) {
            throw std::runtime_error(log.message());
        } else if (next_handler_) {
            next_handler_->handle(log);
        }
    }

private:
    LogMessageHandler* next_handler_ = nullptr;
};

class ErrorHandler : public LogMessageHandler {
public:
    explicit ErrorHandler(const std::filesystem::path& filepath) : filepath_(filepath) {
        std::ofstream ofs(filepath_);
        if (ofs.is_open()) {
            ofs.close();
        }
    }

    void setNextHandler(LogMessageHandler* next_handler) override {
        next_handler_ = next_handler;
    }
    void handle(const LogMessage& log) override {
        if (log.type() == LogMessageType::Error) {
            std::ofstream ofs(filepath_);
            if (ofs.is_open()) {
                ofs << log.message() << std::endl;
                ofs.close();
            }
        } else if (next_handler_) {
            next_handler_->handle(log);
        }
    }

private:
    LogMessageHandler* next_handler_ = nullptr;
    std::filesystem::path filepath_;
};

class WarningHandler : public LogMessageHandler {
public:
    void setNextHandler(LogMessageHandler* next_handler) override {
        next_handler_ = next_handler;
    }
    void handle(const LogMessage& log) override {
        if (log.type() == LogMessageType::Warning) {
            std::cerr << log.message() << std::endl;
        } else if (next_handler_) {
            next_handler_->handle(log);
        }
    }

private:
    LogMessageHandler* next_handler_ = nullptr;
};

class UnknownMessageHandler : public LogMessageHandler {
public:
    void setNextHandler(LogMessageHandler* next_handler) override {
        next_handler_ = next_handler;
    }
    void handle(const LogMessage& log) override {
        if (log.type() == LogMessageType::UnknownMessage) {
            throw std::runtime_error("Unprocessed message: " + log.message());
        } else if (next_handler_) {
            next_handler_->handle(log);
        }
    }

private:
    LogMessageHandler* next_handler_ = nullptr;
};

int main() {
    std::filesystem::path p = "C:\\Users\\Fedot\\Desktop\\cpp\\workspace_net\\net_6_3_3_chain_of_responsibility\\error.txt";
    LogMessageHandler* main_handler = new FatalErrorHandler();
    LogMessageHandler* error_h = new ErrorHandler(p);
    LogMessageHandler* warning_h = new WarningHandler();
    LogMessageHandler* unknown_h = new UnknownMessageHandler();

    main_handler->setNextHandler(error_h);
    error_h->setNextHandler(warning_h);
    warning_h->setNextHandler(unknown_h);

    {
        LogMessage log(LogMessageType::UnknownMessage, "some unknown message");
        try {
            main_handler->handle(log);
        } catch (const std::runtime_error& e) {
            std::cout << e.what() << std:: endl;
        }
    }
    {
        LogMessage log(LogMessageType::Warning, "real warning");
        main_handler->handle(log);
    }
    {
        LogMessage log(LogMessageType::Error, "some_error");
        main_handler->handle(log);
        std::ifstream ifs(p);
        std::string m;
        if (ifs.is_open()) {
            ifs >> m;
            ifs.close();
        }
        std::cout << "LogMessageType::Error = " << m << std::endl;
    }
    {
        LogMessage log(LogMessageType::FatalError, "fatal error");
        try {
            main_handler->handle(log);
        } catch (const std::runtime_error& e) {
            std::cout << e.what() << std:: endl;
        }
    }

    delete unknown_h;
    delete warning_h;
    delete error_h;
    delete main_handler;


    return 0;
}
