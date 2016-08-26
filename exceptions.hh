#ifndef EXCEPTION_HH
#define EXCEPTION_HH

#include <exception>
#include <stdexcept>
namespace Errors {
	class FutureImplementationException : public std::logic_error {
	public:
		explicit FutureImplementationException(const std::string& what_arg):std::logic_error("Missing implementation: " + what_arg) {}
	};
	class AlgorithmFailedException : public std::logic_error {
	public:
		explicit AlgorithmFailedException(const std::string& what_arg):std::logic_error("Algorithm failed: " + what_arg) {}
	};
	/*
	  class  : public std::logic_error {
	  public:
	  explicit (const std::string& what_arg):std::logic_error("Problem type: " + what_arg) {}
	  };
	*/

	class MissingInputDataException : public std::runtime_error {
	public:
		explicit MissingInputDataException(const std::string& what_arg):std::runtime_error("Missing input data: " + what_arg) {}
	};
	class BadRunFileException : public std::runtime_error {
	public:
		explicit BadRunFileException(const std::string& what_arg):std::runtime_error("Run file error: " + what_arg) {}
	};

	class AnalysisException : public std::runtime_error {
	public:
		explicit AnalysisException(const std::string& what_arg):std::runtime_error(what_arg) {}
	};
	class LowSimulationAccuracyException : public AnalysisException {
	public:
		explicit LowSimulationAccuracyException(const std::string& what_arg):AnalysisException("Unacceptable inaccuracies: " + what_arg) {};
	};

	class UIException : public std::runtime_error {
	public:
		explicit UIException(const std::string& what_arg):std::runtime_error("UI Exception: " + what_arg) {}
	};

	class VectorOutOfBoundsException : public std::runtime_error {
	public:
		explicit VectorOutOfBoundsException(const std::string& what_arg):std::runtime_error("Vector index out of bounds: " + what_arg) {}
	  };
	
	/*
	  class  : public std::runtime_error {
	  public:
	  explicit (const std::string& what_arg):std::runtime_error("Problem type: " + what_arg) {}
	  };
	*/
}

#endif
