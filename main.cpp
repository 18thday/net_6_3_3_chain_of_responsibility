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
    void setNextHandler(LogMessageHandler* next_handler) {
        next_handler_ = next_handler;
    }
    void handle(const LogMessage& log) {
        if (log.type() == getLogMessageType()) {
            operate(log);
        } else if (next_handler_) {
            next_handler_->handle(log);
        }
    }

private:
    LogMessageHandler* next_handler_ = nullptr;

    virtual void operate(const LogMessage& log) const = 0;
    virtual LogMessageType getLogMessageType() const = 0;
};

class FatalErrorHandler : public LogMessageHandler {
private:
    LogMessageHandler* next_handler_ = nullptr;

    void operate(const LogMessage& log) const override {
        throw std::runtime_error(log.message());
    }

    LogMessageType getLogMessageType() const override {
        return LogMessageType::FatalError;
    }
};

class ErrorHandler : public LogMessageHandler {
public:
    explicit ErrorHandler(const std::filesystem::path& filepath) : filepath_(filepath) {
        std::ofstream ofs(filepath_);
        if (ofs.is_open()) {
            ofs.close();
        }
    }

private:
    std::filesystem::path filepath_;

    void operate(const LogMessage& log) const override {
        std::ofstream ofs(filepath_);
        if (ofs.is_open()) {
            ofs << log.message() << std::endl;
            ofs.close();
        }
    }

    LogMessageType getLogMessageType() const override{
        return LogMessageType::Error;
    }
};

class WarningHandler : public LogMessageHandler {
private:
    LogMessageHandler* next_handler_ = nullptr;

    void operate(const LogMessage& log) const override {
        std::cerr << log.message() << std::endl;
    }

    LogMessageType getLogMessageType() const override{
        return LogMessageType::Warning;
    }
};

class UnknownMessageHandler : public LogMessageHandler {
private:
    LogMessageHandler* next_handler_ = nullptr;

    void operate(const LogMessage& log) const override {
        throw std::runtime_error("Unprocessed message: " + log.message());
    }

    LogMessageType getLogMessageType() const override{
        return LogMessageType::UnknownMessage;
    }
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
