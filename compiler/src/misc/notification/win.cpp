#ifdef _WIN32

#include <Windows.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/base.h>

namespace notification
{
static std::wstring utf8_to_wide(std::string_view str)
{
  if (str.empty()) return {};

  const int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);

  if (size <= 0) return {};

  std::wstring result(size, L'\0');

  MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), size);

  return result;
}

void win_notify(std::string_view title, std::string_view msg, bool success) noexcept
{
  try {
    winrt::init_apartment();

    using namespace winrt;
    using namespace Windows::Data::Xml::Dom;
    using namespace Windows::UI::Notifications;

    const auto         wtitle = utf8_to_wide(title);
    const std::wstring prefix = success ? L"✓ " : L"✕ ";
    const auto         wmsg   = utf8_to_wide(msg);


    XmlDocument xml;

    xml.LoadXml(
        LR"(
                    <toast>
                        <visual>
                            <binding template="ToastGeneric">
                                <text></text>
                                <text></text>
                            </binding>
                        </visual>
                    </toast>
                )");

    auto textNodes = xml.GetElementsByTagName(L"text");

    textNodes.Item(0).AppendChild(xml.CreateTextNode(prefix + wtitle));

    textNodes.Item(1).AppendChild(xml.CreateTextNode(wmsg));

    ToastNotification toast{xml};

    // Ton AppUserModelID
    auto notifier = ToastNotificationManager::CreateToastNotifier(L"TolzaCompiler");

    notifier.Show(toast);
  } catch (...) {
    // Never failed
  }
}
} // namespace notification

#endif