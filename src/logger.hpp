#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__
#include <cstddef>
#include <iostream>
#include <string>
// #include <format>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <source_location>
#include <thread>
#include <utility>
#include <vector>

namespace yoyo {

template <class T>
class Singleton {
 public:
  static T* getInstance();

 private:
  static std::unique_ptr<T> data_;
  static std::mutex mtx;
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;

 protected:
  Singleton() = default;
  ~Singleton() = default;
};
template <class T>
std::unique_ptr<T> Singleton<T>::data_{nullptr};
template <class T>
std::mutex Singleton<T>::mtx;
template <class T>
T* Singleton<T>::getInstance() {
  if (data_ == nullptr) {
    std::lock_guard<std::mutex> lock(mtx);
    if (data_ == nullptr) {
      data_ = std::make_unique<T>();
    }
  }
  return data_.get();
}

enum class LOGLEVEL { INFO, WARNING, DEBUG, ERROR, FATAL, TRACE };

/* 定义不同的颜色码
const std::string RED = "\033[31m";      // 红色   -> ERROR
const std::string GREEN = "\033[32m";    // 绿色   -> WARNING
const std::string YELLOW = "\033[33m";   // 黄色   -> DEBUG
const std::string BLUE = "\033[34m";     // 蓝色   -> TRACE
const std::string MAGENTA = "\033[35m";  // 品红色 -> FATAL
const std::string CYAN =   "\033[36m";  //  蓝绿   -> INFO
const std::string RESET = "\033[0m";     // 重置颜色
*/
template <class T>
class CirculQueen {
 public:
  CirculQueen() = default;
  /*留个空位作为 空/满的标识位置 */
  explicit CirculQueen(size_t maxSize)
      : _iMaxItem(maxSize + 1), _vQueen(_iMaxItem) {}
  CirculQueen(const CirculQueen&) = default;
  CirculQueen& operator=(const CirculQueen&) = default;
  CirculQueen(CirculQueen&& other) { rvalueCtor(std::move(other)); }
  CirculQueen& operator=(CirculQueen&& other) {
    rvalueCtor(std::move(other));
    return *this;
  }
  ~CirculQueen() = default;
  size_t getNum() const { return (_Tail + _iMaxItem - _Head) % _iMaxItem; }
  void setMaxSize(size_t maxSize) {
    _iMaxItem = maxSize + 1;
    _vQueen.resize(_iMaxItem);
  }
  bool isFull() const { return (_Tail + 1) % _iMaxItem == _Head; }

  bool isEmpty() const { return _Tail == _Head; }

  bool push(T&& item) {
    if (isFull()) return false;
    _vQueen[_Tail] = std::move(item);
    _Tail = (_Tail + 1) % _iMaxItem;
    // todo overCount
    return true;
  }
  const T& front() const { return _vQueen[_Head]; }

  void front_pop() { _Head = (_Head + 1) % _iMaxItem; }

  T pop() {
    T item = front();
    front_pop();
    return item;
  }

 private:
  void rvalueCtor(CirculQueen&& other) {
    _iMaxItem = other._iMaxItem;
    _iOverCount = other._iOverCount;
    _Head = other._Head;
    _Tail = other._Tail;
    _vQueen = std::move(other._vQueen);

    other._iMaxItem = 0;
    other._Head = 0;
    other._Tail = 0;
    other._iOverCount = 0;
  }

 private:
  std::size_t _iMaxItem;
  std::vector<T> _vQueen;
  typename std::vector<T>::size_type _Head = 0;
  typename std::vector<T>::size_type _Tail = 0;
  size_t _iOverCount = 0;
};

// 封装一个buff
template <class T>
class BufferQueen {
 public:
  BufferQueen() : _dataQueen(CirculQueen<T>(_iBufferSize)) {}
  explicit BufferQueen(size_t buffer_size)
      : _dataQueen(CirculQueen<T>(buffer_size)) {}

  void enqueen(T&& item) {
    {
      std::unique_lock<std::mutex> lock(_Mtx);
      _isPushCv.wait(lock, [this]() { return !_dataQueen.isFull(); });
      _dataQueen.push(std::move(item));
    }
    _isPopCv.notify_one();
  }

  void dequeen(T& pop_item) {
    {
      std::unique_lock<std::mutex> lock(_Mtx);
      _isPopCv.wait(lock, [this]() { return !_dataQueen.isEmpty(); });
      pop_item = std::move(_dataQueen.pop());
    }
    _isPushCv.notify_one();
  }

  void dequeen(std::vector<T>& vecBuffer, size_t _batchSize) {
    {
      std::unique_lock<std::mutex> lock(_Mtx);
      _isPopCv.wait_for(lock, std::chrono::milliseconds(1),
                        [this]() { return !_dataQueen.isEmpty(); });
      while (vecBuffer.size() < _batchSize && !_dataQueen.isEmpty()) {
        vecBuffer.emplace_back(_dataQueen.pop());
      }
    }
    _isPushCv.notify_one();
  }
  // todo 对于支持overrun的增加非阻塞机制

  bool isEmpty() { return _dataQueen.isEmpty(); }

  void resize(size_t size) {
    std::unique_lock<std::mutex> lock(_Mtx);
    _dataQueen.setMaxSize(size);
  }

 private:
  constexpr static size_t _iBufferSize{1024 * 10};
  std::mutex _Mtx;
  std::condition_variable _isPushCv;
  std::condition_variable _isPopCv;
  CirculQueen<T> _dataQueen;
};

struct LocationInfo {
 public:
  size_t _Line;
  std::thread::id _threadId;
  std::string _fileName;
  std::string _Function;

  explicit LocationInfo(
      const std::source_location& loc = std::source_location::current())
      : _Line(loc.line()),
        _threadId(std::this_thread::get_id()),
        _fileName(loc.file_name()),
        _Function(loc.function_name()) {}

  LocationInfo(const LocationInfo&) = default;
  LocationInfo& operator=(const LocationInfo&) = default;
  LocationInfo(LocationInfo&&) = default;
  LocationInfo& operator=(LocationInfo&&) = default;
  ~LocationInfo() = default;
};

class Message {
 public:
  explicit Message(LOGLEVEL level, std::string str, LocationInfo&& tLoc)
      : _levle(level),
        _sMsg(std::move(str)),
        _ProduceTime(std::chrono::system_clock::now()),
        _loction(std::move(tLoc)) {}

  Message() = default;
  Message(const Message&) = default;
  Message& operator=(const Message&) = default;
  Message(Message&&) = default;
  Message& operator=(Message&&) = default;
  ~Message() = default;

  void outPutMsg(std::ofstream& ofs, bool isOutFile = true,
                 bool _isColor = true) {
    // 日志格式: [time][level][filename][function][line][msg]
    std::string logMsg =
        "[" + getCurrentTime() + "]" + "[" +
        std::string(LevelFlag[static_cast<int>(_levle)]) + "]" + "[" +
        _loction._fileName + "]" + "[" + _loction._Function + "]" + "[" +
        std::to_string(_loction._Line) + "]" + "[" + _sMsg + "]" + "\n";
    if (isOutFile) {
      // std::string file_name = std::format("./{}.log", _loction._fileName);
      std::string file_name = _loction._fileName.append(".log");
      ofs.open(file_name, std::ios::app);
      if (ofs.is_open()) {
        ofs << logMsg;
        ofs.close();
      }
    }
    if (_isColor) [[likely]] {
      std::cout << LevelColor[static_cast<int>(_levle)] << "["
                << getCurrentTime() << "]"
                << "[" << LevelFlag[static_cast<int>(_levle)] << "]"
                << "[" << _loction._fileName << "]"
                << "[" << _loction._Function << "]"
                << "[" << _loction._Line << "]"
                << "[" << _sMsg << "]" << LevelColor[6] << "\n";
    } else {
      std::cout << logMsg;
    }
    return;
  }

  std::string formatMsg() noexcept {
    std::string str;
    str += "[";
    str += getCurrentTime();
    str += "]";
    str += "[";
    str += LevelFlag[static_cast<int>(_levle)];
    str += "]";
    str += "[";
    str += _loction._fileName;
    str += ":";
    str += _loction._Function;
    str += ":";
    str += std::to_string(_loction._Line);
    str += ":";
    str += _sMsg;
    str += "]";
    return str;
  }

 public:
  std::string getCurrentTime() noexcept {
    auto now = _ProduceTime;
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    auto sectime = std::chrono::duration_cast<std::chrono::seconds>(now_ms);
    int32_t milltime = now_ms.count() % 1000;

    std::time_t timet = sectime.count();
    struct tm curtime;
    localtime_r(&timet, &curtime);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%4d-%02d-%02d %02d:%02d:%02d.%03d",
             curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday,
             curtime.tm_hour, curtime.tm_min, curtime.tm_sec, milltime);
    return std::string(buffer);
  }
  std::string_view getLevelColor() const noexcept {
    return LevelColor[static_cast<int>(_levle)];
  };
  std::string_view getColorReset() const noexcept {
    return LevelColor[static_cast<int>(6)];
  }
  std::string_view getLevelFlag() const noexcept {
    return LevelFlag[static_cast<int>(_levle)];
  }

 private:
  LOGLEVEL _levle;
  std::string _sMsg;
  LocationInfo _loction;
  std::chrono::time_point<std::chrono::system_clock> _ProduceTime;
  constexpr static std::array<std::string_view, 6> LevelFlag{
      "INFO", "WARNING", "DEBUG", "ERROR", "FATAL", "TRACE"};

  constexpr static std::array<std::string_view, 7> LevelColor{
      "\033[36m", "\033[32m", "\033[33m", "\033[31m",
      "\033[35m", "\033[34m", "\033[0m"};
};

class Logger : public Singleton<Logger> {
 public:
  Logger() : _logcof() {
    try {
      initiallize();
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
  ~Logger() {
    _logcof._isStop = true;
    if (_workThread.joinable()) {
      _workThread.join();
    }
  }

 public:
  template <class T>
  void trace(T&& str,
             std::source_location&& loc = std::source_location::current()) {
    log(LOGLEVEL::TRACE, std::forward<T>(str), std::move(loc));
  }
  template <class T>
  void info(T&& str,
            std::source_location&& loc = std::source_location::current()) {
    log(LOGLEVEL::INFO, std::forward<T>(str), std::move(loc));
  }
  template <class T>
  void debug(T&& str,
             std::source_location&& loc = std::source_location::current()) {
    log(LOGLEVEL::DEBUG, std::forward<T>(str), std::move(loc));
  }
  template <class T>
  void error(T&& str,
             std::source_location&& loc = std::source_location::current()) {
    log(LOGLEVEL::ERROR, std::forward<T>(str), std::move(loc));
  }
  template <class T>
  void warn(T&& str,
            std::source_location&& loc = std::source_location::current()) {
    log(LOGLEVEL::WARNING, std::forward<T>(str), std::move(loc));
  }
  template <class T>
  void fatal(T&& str,
             std::source_location&& loc = std::source_location::current()) {
    log(LOGLEVEL::FATAL, std::forward<T>(str), std::move(loc));
  }

 private:
  void log(LOGLEVEL level, std::string&& str, std::source_location&& loc) {
    _buffer.enqueen(
        Message{level, std::move(str), std::move(LocationInfo(loc))});
  }
  void log(LOGLEVEL level, const std::string& str, std::source_location&& loc) {
    _buffer.enqueen(Message{level, str, std::move(LocationInfo(loc))});
  }

  void process() {
    while (!_logcof._isStop || !_buffer.isEmpty()) {
      Message msg;
      _buffer.dequeen(msg);
      msg.outPutMsg(_logout);
    }
  }

  void writeMsgbuffer() {
    for (auto& msg : _writeBuffer) {
      _fileStringBuffer += msg.formatMsg();
      _fileStringBuffer += "\n";
      if (_logcof._isConsle) {
        if (_logcof._isColor) {
          _consoleStringBuffer += msg.getLevelColor();
          _consoleStringBuffer += msg.formatMsg();
          _consoleStringBuffer += msg.getColorReset();
          _consoleStringBuffer += "\n";
        } else {
          _consoleStringBuffer += msg.formatMsg();
          _consoleStringBuffer += "\n";
        }
      }
      if (_logcof._isWritefile &&
          (_fileStringBuffer.size() > _fileCurrentBufferSize)) {
        if (_logout.is_open()) {
          _logout.write(_fileStringBuffer.data(), _fileStringBuffer.size());
          _logout.flush();
          _fileStringBuffer.clear();
        }
        rotateFile();
      }
      if (_logcof._isConsle &&
          _consoleStringBuffer.size() > _consoleCurrentBufferSize) {
        std::ostringstream oss;
        oss << _consoleStringBuffer;
        std::cout << oss.str();
        std::cout.flush();
        _consoleStringBuffer.clear();
      }
    }
  }
  void processBatch(size_t _batchSize) {
    while (!_logcof._isStop || !_buffer.isEmpty()) {
      _buffer.dequeen(_writeBuffer, _batchSize);
      if (!_writeBuffer.empty()) {
        writeMsgbuffer();
        _writeBuffer.clear();
      }
    }
    if (_logcof._isWritefile && !_fileStringBuffer.empty()) {
      if (_logout.is_open()) {
        _logout.write(_fileStringBuffer.data(), _fileStringBuffer.size());
        _logout.flush();
        _fileStringBuffer.clear();
      }
      rotateFile();
    }
    if (_logcof._isConsle && !_consoleStringBuffer.empty()) {
      std::ostringstream oss;
      oss << _consoleStringBuffer;
      std::cout << oss.str();
      std::cout.flush();
      _consoleStringBuffer.clear();
    }
  }

  void rotateFile() {
    if (!_logcof._isRotate) return;
    std::string file_path = getLognName().append(".log");
    int filesize = 0;
    if (std::filesystem::exists(file_path) &&
        std::filesystem::is_regular_file(file_path)) {
      filesize = std::filesystem::file_size(file_path);
    }
    if (filesize < _logcof._fileMaxSize) return;
    _logout.close();
    // generate new filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char timestamp[32];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S",
                  std::localtime(&time));
    std::string new_name = getLognName();
    new_name += "_";
    new_name += std::string(timestamp);
    new_name += ".log";
    if (std::filesystem::exists(file_path)) {
      std::filesystem::rename(file_path, new_name);
    }
    _logout.open(file_path);
  }

 private:
  struct logConf {
    bool _isStop;
    bool _isColor;
    bool _isConsle;
    bool _isWritefile;
    bool _isRotate;
    size_t _fileMaxSize;
    size_t _fileNum;
    std::string _logDirName;
    std::string _logPrefixPath;
    std::string _logFileName;
    logConf()
        : _fileMaxSize(1024 * 1024 * 128),
          _fileNum(10),
          _isConsle(true),
          _isColor(true),
          _isWritefile(true),
          _isRotate(false),
          _logDirName("log"),
          _isStop(false),
          _logPrefixPath("."),
          _logFileName("app") {}
  };

 private:
  void createlogDir() {
    std::string logdir = getLogDirName();
    std::filesystem::path dir(logdir);
    if (!std::filesystem::exists(dir)) {
      std::filesystem::create_directories(dir);
    }
  }
  std::string getLogDirName() const {
    std::string dirpath;
    return dirpath.append(_logcof._logPrefixPath)
        .append("/")
        .append(_logcof._logDirName);
  }
  std::string getLognName() const {
    std::string dirpath = getLogDirName();
    return dirpath.append("/").append(_logcof._logFileName);
  }

 public:
  Logger& setLogDirName(std::string logDirName) {
    _logcof._logDirName = std::move(logDirName);
    return *this;
  }
  Logger& setPrefixPath(std::string prefixPath) {
    _logcof._logPrefixPath = std::move(prefixPath);
    return *this;
  }
  Logger& setLogFileName(std::string logFileName) {
    std::lock_guard<std::mutex> lock(_mtx);
    _logcof._logFileName = std::move(logFileName);
    return *this;
  }

  Logger& setFileMaxSize(size_t fileMaxSize) {
    _logcof._fileMaxSize = fileMaxSize;
    return *this;
  }
  Logger& setFileNum(size_t fileNum) {
    _logcof._fileNum = fileNum;
    return *this;
  }
  Logger& setConsle(bool isConsle) {
    _logcof._isConsle = isConsle;
    return *this;
  }
  Logger& setColor(bool isColor) {
    _logcof._isColor = isColor;
    return *this;
  }
  Logger& setWritefile(bool isWritefile) {
    _logcof._isWritefile = isWritefile;
    return *this;
  }
  Logger& setRotate(bool isRotate) {
    _logcof._isRotate = isRotate;
    return *this;
  }

 private:
  std::ofstream _logout;
  BufferQueen<Message> _buffer;
  std::thread _workThread;
  logConf _logcof;

 private:
  std::mutex _mtx;

  std::string _fileStringBuffer;
  std::string _consoleStringBuffer;
  size_t _fileCurrentBufferSize;
  size_t _consoleCurrentBufferSize;
  thread_local inline static std::vector<Message> _writeBuffer;
  void initiallize() {
    constexpr size_t _iQueenBufferSize = 1 << 13;  // ciculQueen size 1024 * 8
    size_t _batchSize =
        _iQueenBufferSize >> 1;  // batch size circulQueen size / 2
    _buffer.resize(_iQueenBufferSize);
    _writeBuffer.reserve(_batchSize);

    createlogDir();
    _logout.open(getLognName().append(".log"));

    setConsle(false).setRotate(false);

    _fileCurrentBufferSize = _logcof._fileMaxSize >> 1;
    _consoleCurrentBufferSize = _logcof._fileMaxSize >> 2;
    _fileStringBuffer.reserve(_fileCurrentBufferSize + 1024);
    _consoleStringBuffer.reserve(_consoleCurrentBufferSize + 512);
    _workThread = (std::thread(&Logger::processBatch, this, _batchSize));
  }
};

#define LOGI(Msg) yoyo::Logger::getInstance()->info(Msg);
#define LOGD(Msg) yoyo::Logger::getInstance()->debug(Msg);
#define LOGW(Msg) yoyo::Logger::getInstance()->warn(Msg);
#define LOGE(Msg) yoyo::Logger::getInstance()->error(Msg);
#define LOGF(Msg) yoyo::Logger::getInstance()->fatal(Msg);
#define LOGT(Msg) yoyo::Logger::getInstance()->trace(Msg);

}  // namespace yoyo

#endif