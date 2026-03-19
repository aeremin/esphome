#include "pn7160_i2c.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace pn7160_i2c {

static const char *const TAG = "pn7160_i2c";

uint8_t PN7160I2C::read_nfcc(nfc::NciMessage &rx, const uint16_t timeout) {
  if (this->wait_for_irq_(timeout) != nfc::STATUS_OK) {
    ESP_LOGW(TAG, "read_nfcc_() timeout waiting for IRQ");
    return nfc::STATUS_FAILED;
  }

  rx.get_message().resize(nfc::NCI_PKT_HEADER_SIZE);
  if (!this->read_bytes_raw(rx.get_message().data(), nfc::NCI_PKT_HEADER_SIZE)) {
    return nfc::STATUS_FAILED;
  }

  uint8_t length = rx.get_payload_size();
  if (length > 0) {
    rx.get_message().resize(length + nfc::NCI_PKT_HEADER_SIZE);
    if (!this->read_bytes_raw(rx.get_message().data() + nfc::NCI_PKT_HEADER_SIZE, length)) {
      return nfc::STATUS_FAILED;
    }
  }
  // #region agent log
  if (this->irq_pin_->digital_read()) {
    ESP_LOGD(TAG, "[DBG-57126c-POSTFIX] post-read IRQ still HIGH (more data pending), read OK: payload=%u", length);
  }
  // #endregion
  return nfc::STATUS_OK;
}

uint8_t PN7160I2C::write_nfcc(nfc::NciMessage &tx) {
  auto encoded = tx.encode();
  if (this->write(encoded.data(), encoded.size()) == i2c::ERROR_OK) {
    return nfc::STATUS_OK;
  }
  return nfc::STATUS_FAILED;
}

void PN7160I2C::dump_config() {
  PN7160::dump_config();
  LOG_I2C_DEVICE(this);
}

}  // namespace pn7160_i2c
}  // namespace esphome
