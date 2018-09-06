#include <thread>
#include "IOContextPool.h"

IOContextPool::IOContextPool(std::size_t pool_size)
  : next_io_context_(0)
{
  if (pool_size == 0)
  {
      throw std::runtime_error("IOContextPool size is 0");
  }

  // Give all the io_contexts work to do so that their run() functions will not
  // exit until they are explicitly stopped.
  for (std::size_t i = 0; i < pool_size; ++i)
  {
    io_context_ptr io_context(new boost::asio::io_context);
    io_contexts_.push_back(io_context);
    work_.push_back(boost::asio::make_work_guard(*io_context));
  }
}

void IOContextPool::Run()
{
    // Create a pool of threads to run all of the io_contexts.
    std::vector<std::shared_ptr<std::thread> > threads;
    for (std::size_t i = 0; i < io_contexts_.size(); ++i)
    {
        std::shared_ptr<std::thread> thread(new std::thread(
              boost::bind(&boost::asio::io_context::run, io_contexts_[i])));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
    {
        threads[i]->join();
    }
}

void IOContextPool::Stop()
{
    // Explicitly stop all io_contexts.
    for (std::size_t i = 0; i < io_contexts_.size(); ++i)
    {
        io_contexts_[i]->stop();
    }
}

boost::asio::io_context& IOContextPool::get_io_context()
{
    // Use a round-robin scheme to choose the next io_context to use.
    boost::asio::io_context& io_context = *io_contexts_[next_io_context_];
    ++next_io_context_;
    if (next_io_context_ == io_contexts_.size())
    {
        next_io_context_ = 0;
    }
    return io_context;
}
