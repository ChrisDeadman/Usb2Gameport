#include "HIDDescriptionParser.h"

int HIDDescriptionParser::getUsageDescriptionCount() {
  return usageDescriptionCount;
}

HIDReportUsageDescription * HIDDescriptionParser::getUsageDescription(int idx) {
  return &usageDescriptions[idx % usageDescriptionCount];
}

void HIDDescriptionParser::parseHidDataBlock(HIDReportUsageDescription * description, uint8_t * input, int * inputIdx, uint8_t * bitIdx, uint8_t * output, int * outputSize) {
  int maxSize = *outputSize;
  *outputSize = 0;

  // placeholder -> ignore incomplete  byte
  if (description->placeholder) {
    *bitIdx = 0;
    *inputIdx = (*inputIdx) + 1;
    return;
  }

  for (int idx = 0; idx < description->reportCount; idx++) {
    uint16_t curValue = 0;

    for (uint8_t bitsRemaining = description->reportSize; bitsRemaining > 0; bitsRemaining--) {
      if (input[*inputIdx] & (1 << *bitIdx)) {
        curValue |= 1 << (description->reportSize - bitsRemaining);
      }
      if (*bitIdx >= 7) {
        *bitIdx = 0;
        *inputIdx = (*inputIdx) + 1;
      } else {
        *bitIdx = *bitIdx + 1;
      }
    }
    if (*outputSize < maxSize) {
      if (description->usage != 0x39) { /* do not scale HATs */
        if (description->logicalMaximum >= description->logicalMinimum) {
          output[*outputSize] = (curValue - description->logicalMinimum) * (256.0f / ((description->logicalMaximum - description->logicalMinimum) + 1));
        } else {
          output[*outputSize] = (description->logicalMinimum - curValue) * (256.0f / ((description->logicalMinimum - description->logicalMaximum) + 1));
        }
      } else {
        output[*outputSize] = min(curValue, 0xFF);
      }
      *outputSize = (*outputSize) + 1;
    }
  }
}

void HIDDescriptionParser::parseDeviceDescriptor(uint8_t address, uint8_t endpoint) {
  static const int BUFFER_SIZE = 128;
  uint8_t receiveBuffer[BUFFER_SIZE];
  uint8_t hidRepDescBuffer[BUFFER_SIZE];
  uint8_t rcode;

  // reset usage descriptors
  usageDescriptionCount = 0;
  for (int idx = 0; idx < MAX_USAGE_DESCRIPTIONS; idx++) {
    memset(&usageDescriptions[idx], 0, sizeof(HIDReportUsageDescription));
  }

  rcode = usb->getDevDescr(address, endpoint, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t*)receiveBuffer);
  if (rcode) return;

  USB_DEVICE_DESCRIPTOR * devDesc = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(receiveBuffer);
  HID_JOYSTICK_DEBUG(println("USB_DEVICE_DESCRIPTOR"));
  HID_JOYSTICK_DEBUG(print("bLength=")); HID_JOYSTICK_DEBUG(println(devDesc->bLength, HEX));
  HID_JOYSTICK_DEBUG(print("bDescriptorType=")); HID_JOYSTICK_DEBUG(println(devDesc->bDescriptorType, HEX));
  HID_JOYSTICK_DEBUG(print("bDeviceClass=")); HID_JOYSTICK_DEBUG(println(devDesc->bDeviceClass, HEX));
  HID_JOYSTICK_DEBUG(print("idVendor=")); HID_JOYSTICK_DEBUG(println(devDesc->idVendor, HEX));
  HID_JOYSTICK_DEBUG(print("idProduct=")); HID_JOYSTICK_DEBUG(println(devDesc->idProduct, HEX));
  HID_JOYSTICK_DEBUG(print("bNumConfigurations=")); HID_JOYSTICK_DEBUG(println(devDesc->bNumConfigurations, HEX));
  HID_JOYSTICK_DEBUG(println());

  for (uint8_t configIdx = 0; configIdx < devDesc->bNumConfigurations; configIdx++) {
    rcode = usb->getConfDescr(address, endpoint, sizeof(USB_CONFIGURATION_DESCRIPTOR), configIdx, (uint8_t*)receiveBuffer);
    if (rcode) break;

    USB_CONFIGURATION_DESCRIPTOR * confDesc = reinterpret_cast<USB_CONFIGURATION_DESCRIPTOR*>(receiveBuffer);
    HID_JOYSTICK_DEBUG(println("USB_CONFIGURATION_DESCRIPTOR"));
    HID_JOYSTICK_DEBUG(print("bLength=")); HID_JOYSTICK_DEBUG(println(confDesc->bLength, HEX));
    HID_JOYSTICK_DEBUG(print("bDescriptorType=")); HID_JOYSTICK_DEBUG(println(confDesc->bDescriptorType, HEX));
    HID_JOYSTICK_DEBUG(print("wTotalLength=")); HID_JOYSTICK_DEBUG(println(confDesc->wTotalLength, HEX));
    HID_JOYSTICK_DEBUG(print("bNumInterfaces=")); HID_JOYSTICK_DEBUG(println(confDesc->bNumInterfaces, HEX));
    HID_JOYSTICK_DEBUG(println());

    uint16_t totalLength = min(confDesc->wTotalLength, BUFFER_SIZE);
    rcode = usb->getConfDescr(address, endpoint, totalLength, configIdx, receiveBuffer);
    if (rcode) break;

    uint8_t hidInterfaceNumber = 0;

    USB_INTERFACE_DESCRIPTOR * interfaceDesc;
    USB_HID_DESCRIPTOR * hidDesc;
    HID_CLASS_DESCRIPTOR_LEN_AND_TYPE * hidClassDesc;

    int bufferIdx = confDesc->bLength;
    while ((bufferIdx + 2) < totalLength) {
      uint8_t descLen = receiveBuffer[bufferIdx];
      uint8_t descType = receiveBuffer[bufferIdx+1];
      if ((bufferIdx + descLen) > totalLength) break; // not enough memory

      switch (descType) {
        case USB_DESCRIPTOR_INTERFACE: {
          interfaceDesc = reinterpret_cast<USB_INTERFACE_DESCRIPTOR*>(&receiveBuffer[bufferIdx]);
          HID_JOYSTICK_DEBUG(println("USB_INTERFACE_DESCRIPTOR"));
          HID_JOYSTICK_DEBUG(print("bLength=")); HID_JOYSTICK_DEBUG(println(interfaceDesc->bLength, HEX));
          HID_JOYSTICK_DEBUG(print("bDescriptorType=")); HID_JOYSTICK_DEBUG(println(interfaceDesc->bDescriptorType, HEX));
          HID_JOYSTICK_DEBUG(print("bInterfaceNumber=")); HID_JOYSTICK_DEBUG(println(interfaceDesc->bInterfaceNumber, HEX));
          HID_JOYSTICK_DEBUG(print("bNumEndpoints=")); HID_JOYSTICK_DEBUG(println(interfaceDesc->bNumEndpoints, HEX));
          HID_JOYSTICK_DEBUG(print("bInterfaceClass=")); HID_JOYSTICK_DEBUG(println(interfaceDesc->bInterfaceClass, HEX));
          HID_JOYSTICK_DEBUG(println());
          if (interfaceDesc->bInterfaceClass == USB_CLASS_CODE_HID) {
            hidInterfaceNumber = interfaceDesc->bInterfaceNumber;
          }
          break;
        }
        case HID_DESCRIPTOR_HID: {
          hidDesc = reinterpret_cast<USB_HID_DESCRIPTOR*>(&receiveBuffer[bufferIdx]);
          HID_JOYSTICK_DEBUG(println("USB_HID_DESCRIPTOR"));
          HID_JOYSTICK_DEBUG(print("bLength=")); HID_JOYSTICK_DEBUG(println(hidDesc->bLength, HEX));
          HID_JOYSTICK_DEBUG(print("bDescriptorType=")); HID_JOYSTICK_DEBUG(println(hidDesc->bDescriptorType, HEX));
          HID_JOYSTICK_DEBUG(print("bNumDescriptors=")); HID_JOYSTICK_DEBUG(println(hidDesc->bNumDescriptors, HEX));
          HID_JOYSTICK_DEBUG(println());

          for (uint8_t hidClassDescIdx = 0; hidClassDescIdx < hidDesc->bNumDescriptors; hidClassDescIdx++) {
            hidClassDesc = reinterpret_cast<HID_CLASS_DESCRIPTOR_LEN_AND_TYPE*>(&receiveBuffer[bufferIdx+6+(hidClassDescIdx* sizeof(HID_CLASS_DESCRIPTOR_LEN_AND_TYPE))]);
            HID_JOYSTICK_DEBUG(println("HID_CLASS_DESCRIPTOR"));
            HID_JOYSTICK_DEBUG(print("bDescrType=")); HID_JOYSTICK_DEBUG(println(hidClassDesc->bDescrType, HEX));
            HID_JOYSTICK_DEBUG(print("wDescriptorLength=")); HID_JOYSTICK_DEBUG(println(hidClassDesc->wDescriptorLength, HEX));
            HID_JOYSTICK_DEBUG(println());

            if (hidClassDesc->bDescrType != HID_DESCRIPTOR_REPORT) {
              break;
            }

            uint8_t requestType = 0x81; // read (1) using a standard (00) transmission and query an interface (00001)
            uint8_t request = GET_DESCRIPTOR;
            uint16_t responseLen = min(hidClassDesc->wDescriptorLength, BUFFER_SIZE);
            rcode = usb->ctrlReq(address, endpoint, requestType, request, hidInterfaceNumber, HID_DESCRIPTOR_REPORT, hidInterfaceNumber, responseLen, 0, hidRepDescBuffer, NULL);
            if (rcode) break;

            HID_JOYSTICK_DEBUG(println("HID_DESCRIPTOR_REPORT (BUFFER)"));
            for (uint16_t i = 0; i < responseLen; i++) {
              HID_JOYSTICK_DEBUG(print(hidRepDescBuffer[i], HEX));
              if (((i+1) % 16) == 0) {
                HID_JOYSTICK_DEBUG(println());
              } else {
                HID_JOYSTICK_DEBUG(print(" "));
              }
            }
            HID_JOYSTICK_DEBUG(println());

            // Parse HID Report Descriptor
            uint8_t usagePage = 0;
            uint8_t usage = 0;
            uint16_t logicalMinimum = 0;
            uint16_t logicalMaximum = 0;
            uint16_t physicalMinimum = 0;
            uint16_t physicalMaximum = 0;
            uint8_t usageMinimum = 0;
            uint8_t usageMaximum = 0;
            uint8_t reportSize = 0;
            uint8_t reportCount = 1;
            for (int idx = 0; idx < responseLen; idx += 2) {
              uint8_t key = hidRepDescBuffer[idx];
              uint8_t value = hidRepDescBuffer[idx+1];
              switch (key) {
                case 0x05: {// usage Page
                  usagePage = value;
                  break;
                }
                case 0x09: { // usage
                  usage = value;
                  break;
                }
                case 0x15: { // logical minimum
                  logicalMinimum = value;
                  break;
                }
                case 0x16: { // 3-byte version
                  logicalMinimum = value | (hidRepDescBuffer[idx+2] << 8);
                  idx++;
                  break;
                }
                case 0x25: { // logical maximum
                  logicalMaximum = value;
                  break;
                }
                case 0x26: { // 3-byte version
                  logicalMaximum = value | (hidRepDescBuffer[idx+2] << 8);
                  idx++;
                  break;
                }
                case 0x35: { // physical minimum
                  physicalMinimum = value;
                  break;
                }
                case 0x36: { // 3-byte version
                  physicalMinimum = value | (hidRepDescBuffer[idx+2] << 8);
                  idx++;
                  break;
                }
                case 0x45: { // physical maximum
                  physicalMaximum = value;
                  break;
                }
                case 0x46: { // 3-byte version
                  physicalMaximum = value | (hidRepDescBuffer[idx+2] << 8);
                  idx++;
                  break;
                }
                case 0x19: { // usage minimum
                  usageMinimum = value;
                  break;
                }
                case 0x29: { // usage maximum
                  usageMaximum = value;
                  break;
                }
                case 0x75: { // report size
                  reportSize = value;
                  break;
                }
                case 0x95: { // report count
                  reportCount = value;
                  break;
                }
                case 0x81: {// input
                  HID_JOYSTICK_DEBUG(println("USB HID USAGE DESCRIPTOR"));
                  HID_JOYSTICK_DEBUG(print("usagePage=")); HID_JOYSTICK_DEBUG(println(usagePage, HEX));
                  HID_JOYSTICK_DEBUG(print("usage=")); HID_JOYSTICK_DEBUG(println(usage, HEX));
                  HID_JOYSTICK_DEBUG(print("logicalMinimum=")); HID_JOYSTICK_DEBUG(println(logicalMinimum));
                  HID_JOYSTICK_DEBUG(print("logicalMaximum=")); HID_JOYSTICK_DEBUG(println(logicalMaximum));
                  HID_JOYSTICK_DEBUG(print("physicalMinimum=")); HID_JOYSTICK_DEBUG(println(physicalMinimum));
                  HID_JOYSTICK_DEBUG(print("physicalMaximum=")); HID_JOYSTICK_DEBUG(println(physicalMaximum));
                  HID_JOYSTICK_DEBUG(print("usageMinimum=")); HID_JOYSTICK_DEBUG(println(usageMinimum));
                  HID_JOYSTICK_DEBUG(print("usageMaximum=")); HID_JOYSTICK_DEBUG(println(usageMaximum));
                  HID_JOYSTICK_DEBUG(print("reportSize=")); HID_JOYSTICK_DEBUG(println(reportSize));
                  HID_JOYSTICK_DEBUG(print("reportCount=")); HID_JOYSTICK_DEBUG(println(reportCount));
                  HID_JOYSTICK_DEBUG(print("placeholder=")); HID_JOYSTICK_DEBUG(println(value & 1));
                  HID_JOYSTICK_DEBUG(println());

                  if (usageDescriptionCount < MAX_USAGE_DESCRIPTIONS) {
                    usageDescriptions[usageDescriptionCount].usagePage = usagePage;
                    usageDescriptions[usageDescriptionCount].usage = usage;
                    usageDescriptions[usageDescriptionCount].logicalMinimum = logicalMinimum;
                    usageDescriptions[usageDescriptionCount].logicalMaximum = logicalMaximum;
                    usageDescriptions[usageDescriptionCount].physicalMinimum = physicalMinimum;
                    usageDescriptions[usageDescriptionCount].physicalMaximum = physicalMaximum;
                    usageDescriptions[usageDescriptionCount].usageMinimum = usageMinimum;
                    usageDescriptions[usageDescriptionCount].usageMaximum = usageMaximum;
                    usageDescriptions[usageDescriptionCount].reportSize = reportSize;
                    usageDescriptions[usageDescriptionCount].reportCount = reportCount;
                    usageDescriptions[usageDescriptionCount].placeholder = value & 1;
                    usageDescriptionCount++;
                  }

                  usage = 0;
                  usageMinimum = 0;
                  usageMaximum = 0;
                  reportSize = 0;
                  reportCount = 1;
                  break;
                }
              }
            }
          }
          break;
        }
        default: { // ignore other descriptors
          break;
        }
      }
      bufferIdx += descLen;
    }
  }
}
