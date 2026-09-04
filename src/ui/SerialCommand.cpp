#include "ui/SerialCommand.h"

void SerialCommand::poll() {
    // TODO(serial): read available chars into line_; on '\n' call
    // dispatch() unless overflow_ is set, then reset len_ and overflow_.
    // Ignore '\r'. Set overflow_ when a line exceeds sizeof line_ - 1.
}

void SerialCommand::dispatch() {
    // TODO(serial): terminate line_, skip leading spaces, split first word
    // from the rest, call handler_(cmd, arg).
}
