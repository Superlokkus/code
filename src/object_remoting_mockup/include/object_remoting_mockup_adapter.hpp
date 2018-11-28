/*! @file object_remoting_mockup_adapter.hpp
 *
 */

#ifndef MKDT_OBJECT_REMOTING_MOCKUP_ADAPTER_HPP
#define MKDT_OBJECT_REMOTING_MOCKUP_ADAPTER_HPP

#include <object_remoting_mockup.hpp>
#include <registry.hpp>

namespace mkdt {
namespace object_remoting_mockup {

template<typename StubT>
struct stub_adapter : StubT, registry::receiver {

    stub_adapter(mkdt::registry &registry) :
            StubT(), registry::receiver(),
            registry_(registry) {
        this->set_sending_callback([this](auto message) {
            auto message_ptr = std::make_shared<std::string>(message);
            this->send_via_registry(message_ptr);
        });
    }

    void receive(const std::string &message, const object_identifier &sender) override {
        this->handle_incoming_data(message);
    }

private:
    registry registry_;
    object_identifier last_sender_;

    void send_via_registry(std::shared_ptr<std::string> message) {
        this->registry_.send_message_to_object(last_sender_, *message, [message]() {});
    }
};

}
}

#endif //MKDT_OBJECT_REMOTING_MOCKUP_ADAPTER_HPP
