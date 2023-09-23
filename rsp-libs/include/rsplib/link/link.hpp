// eventhandler
class link {
 public:
  virtual void on_closed() = 0;
  virtual void on_recv() = 0;
  template <typename Message>
  void send(Message& msg) {}
};
