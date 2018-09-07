#ifndef _OLDPART_IOCONTEXT_POOL_H
#define _OLDPART_IOCONTEXT_POOL_H
#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/noncopyable.hpp>
/// A pool of io_context objects.
class IOContextPool
  : private boost::noncopyable
{
public:
  /// Construct the io_context pool.
  explicit IOContextPool(std::size_t pool_size);

  /// Run all io_context objects in the pool.
  void Run();

  /// Stop all io_context objects in the pool.
  void Stop();

  /// Get an io_context to use.
  boost::asio::io_context& GetIOContext();

private:
  typedef std::shared_ptr<boost::asio::io_context> io_context_ptr;
  typedef boost::asio::executor_work_guard<
    boost::asio::io_context::executor_type> io_context_work;

  /// The pool of io_contexts.
  std::vector<io_context_ptr> io_contexts_;

  /// The work that keeps the io_contexts running.
  std::vector<io_context_work> work_;

  /// The next io_context to use for a connection.
  std::size_t next_io_context_;
};

#endif /* end of include guard: _OLDPART_IOCONTEXT_POOL_H */
