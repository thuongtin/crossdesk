import type { App } from "@modelcontextprotocol/ext-apps";
import "./viewer.css";
import { AudioPlayer } from "./audio-player.js";
import { HumanInput } from "./human-input.js";
import { VideoPlayer, type FrameInfo } from "./video-player.js";
import { PreviewPlayer } from "./preview-player.js";

declare const CROSSDESK_APPS: boolean;
declare const CROSSDESK_LOGO: string;
type Data = Record<string, any>;
type Invoke = (name: string, args?: Data) => Promise<Data>;
const icons = {
  monitor: '<rect x="3" y="4" width="18" height="12" rx="2"/><path d="M8 20h8m-4-4v4"/>',
  info: '<circle cx="12" cy="12" r="9"/><path d="M12 11v6m0-10v1"/>',
  volume: '<path d="m11 4-6 5H2v6h3l6 5V4Zm4 4a6 6 0 0 1 0 8m3-11a10 10 0 0 1 0 14"/>',
  mouse: '<rect x="6" y="2" width="12" height="20" rx="6"/><path d="M12 2v6"/>',
  pause: '<path d="M8 5v14m8-14v14"/>',
  power: '<path d="M12 2v10m6-7a9 9 0 1 1-12 0"/>',
  expand: '<path d="M8 3H3v5m13-5h5v5M3 16v5h5m13-5v5h-5"/>',
  arrow: '<path d="M4 12h16m-6-6 6 6-6 6"/>',
  more: '<circle cx="5" cy="12" r="1"/><circle cx="12" cy="12" r="1"/><circle cx="19" cy="12" r="1"/>',
};
const icon = (name: keyof typeof icons) => `<svg class="icon icon-${name}" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">${icons[name]}</svg>`;
const root = document.querySelector<HTMLDivElement>("#app")!;
document.documentElement.lang = "vi";
root.innerHTML = `
<header class="app-header">
  <div class="brand"><img class="brand-logo" src="${CROSSDESK_LOGO}" alt="" width="24" height="24"><h1>CrossDesk</h1><span class="brand-context">Phiên Agent</span></div>
  <div class="header-right"><button id="details-toggle" class="quiet" aria-expanded="false" aria-controls="sidebar">${icon("info")}<span id="details-label">Chi tiết phiên</span></button><span class="connection" id="connection" role="status"><span id="connection-label">Đang chờ kết nối</span></span></div>
</header>
<div id="error" role="alert" hidden></div>
<main>
  <button id="details-backdrop" class="details-backdrop" aria-label="Đóng chi tiết phiên" tabindex="-1" hidden></button>
  <aside id="sidebar" aria-labelledby="sidebar-title">
    <div class="sidebar-heading"><h2 id="sidebar-title">Cấp quyền kết nối cho Agent</h2><button id="details-close" class="quiet icon-button" aria-label="Đóng chi tiết phiên">×</button></div>
    <div id="connect-form" role="form" aria-label="Kết nối máy tính từ xa">
      <p class="authorization-note">Sau khi bạn chọn thiết bị và cấp quyền, Agent sẽ tiếp tục tác vụ. Bạn có thể xem hình ảnh hoặc tạm dừng điều khiển tại đây.</p>
      <fieldset id="connection-fields">
        <label for="device">Thiết bị</label><select id="device"><option value="">Kết nối thiết bị mới</option></select>
        <div id="new-device">
          <label for="device-id">ID thiết bị</label><input id="device-id" inputmode="numeric" autocomplete="off" placeholder="Nhập ID thiết bị 9 chữ số" maxlength="11">
        </div>
        <label for="password">Mật khẩu truy cập</label><div class="password-field"><input id="password" type="password" autocomplete="off" maxlength="256" placeholder="Nhập mật khẩu truy cập từ xa"><button type="button" id="password-toggle" aria-label="Hiện mật khẩu" aria-pressed="false">Hiện</button></div>
        <label for="mode">Phạm vi cấp quyền</label><select id="mode" aria-describedby="mode-description"><option value="observe">Chỉ xem hình ảnh</option><option value="control">Cho phép Agent thao tác</option></select>
        <p class="field-hint" id="mode-description">Agent có thể xem màn hình, không thể thao tác chuột và bàn phím.</p>
        <details class="preferences"><summary>Lưu và cấp quyền</summary>
          <div id="new-device-label"><label for="label">Ghi chú thiết bị <span class="optional">Không bắt buộc</span></label><input id="label" placeholder="Ví dụ: Máy tính công việc" maxlength="80"></div>
          <div class="checks">
            <label><input id="save" type="checkbox"><span>Lưu thiết bị</span></label>
            <label><input id="remember" type="checkbox" disabled><span>Nhớ mật khẩu<small>Lưu trong kho xác thực hệ thống trên máy này</small></span></label>
            <label><input id="remember-unlock" type="checkbox" disabled><span>Lưu mật khẩu khóa màn hình và cho phép tự mở khóa<small>Lưu riêng trong kho xác thực hệ thống trên máy này</small></span></label>
            <label><input id="authorize" type="checkbox" disabled><span>Cho phép tự kết nối lần sau<small>Dùng lại phạm vi cấp quyền lần này</small></span></label>
          </div>
          <div id="saved-unlock-field" hidden><label for="saved-unlock-password">Mật khẩu khóa màn hình hệ thống</label><input id="saved-unlock-password" type="password" autocomplete="off" maxlength="256"></div>
        </details>
        <details id="advanced"><summary>Cài đặt nâng cao</summary>
          <label for="server">Máy chủ</label><input id="server" value="api.crossdesk.cn" autocomplete="off">
          <div class="row"><div><label for="port">Cổng</label><input id="port" type="number" min="1" max="65535" value="9099"></div><div><label for="display">Chỉ số màn hình</label><input id="display" type="number" min="0" max="63" value="0"></div></div>
        </details>
      </fieldset>
      <button id="connect" type="button" class="primary" disabled><span id="connect-label">Cấp quyền và kết nối</span>${icon("arrow")}</button>
      <p class="privacy">Mật khẩu không được gửi vào hội thoại Agent.</p>
    </div>
    <div id="session-card" hidden>
      <div class="device-card"><div class="device-icon">${icon("monitor")}</div><div class="device-identity"><strong id="session-device">Máy tính từ xa</strong><span id="session-platform">Đã kết nối</span></div></div>
      <dl class="session-details"><div id="session-device-id-row" hidden><dt>ID thiết bị</dt><dd id="session-device-id"></dd></div><div><dt>Quyền truy cập</dt><dd id="session-access"></dd></div><div><dt>Màn hình</dt><dd id="session-display"></dd></div><div><dt>Trạng thái Agent</dt><dd id="session-agent"></dd></div><div><dt>Mật khẩu khóa màn hình</dt><dd id="session-unlock-credential"></dd></div><div id="session-unlock-row" hidden><dt>Trạng thái mở khóa</dt><dd class="session-unlock-details"><span id="session-unlock-state"></span></dd></div></dl>
      <label class="check"><input id="recovery" type="checkbox"><span>Tự khôi phục khi mất kết nối</span></label>
      <p class="session-note">Đóng trang không ngắt kết nối.</p>
    </div>
  </aside>
  <section class="workspace" aria-label="Hình ảnh từ xa và điều khiển phiên">
    <div class="viewer-card">
      <div class="viewer-toolbar">
        <div class="viewer-title"><h2 id="device-name">Hình ảnh từ xa</h2><span id="agent-state" role="status">Đang chờ cấp quyền</span></div>
        <div class="viewer-controls">
          <button id="return-agent" class="soft" hidden>${icon("arrow")}<span>Trả quyền cho Agent</span></button>
          <button id="pause" title="Tạm dừng điều khiển của Agent" hidden disabled>${icon("pause")}<span id="pause-label">Tạm dừng Agent</span></button>
          <button id="more-toggle" class="quiet icon-button" aria-label="Tùy chọn phiên" title="Tùy chọn phiên" aria-expanded="false" aria-controls="session-menu" aria-haspopup="dialog">${icon("more")}</button>
        </div>
      </div>
      <div id="attention" class="attention" role="status" hidden><span>Agent đang chờ mở khóa hệ thống</span><button id="unlock-open" type="button" hidden>Nhập mật khẩu</button></div>
      <div class="screen" id="screen">
        <canvas id="frame" role="img" aria-label="Hình ảnh trực tiếp của desktop từ xa" hidden></canvas>
        <canvas id="agent-cursor" aria-hidden="true"></canvas>
        <textarea id="remote-input" aria-label="Nhập bàn phím từ xa" autocomplete="off" autocapitalize="off" spellcheck="false"></textarea>
        <div class="control-halo" aria-hidden="true"></div>
        <div id="empty"><div class="empty-icon">${icon("monitor")}</div><h3 id="empty-title">Chưa kết nối thiết bị</h3><p id="empty-description">Chọn thiết bị và nhập mật khẩu truy cập.</p></div>
        <div class="stale" id="stale" role="status" hidden>Hình ảnh tạm chưa cập nhật</div>
      </div>
      <div class="telemetry"><span id="video-state">Chờ kết nối</span><div class="stream-metrics"><span id="resolution">—</span><span id="fps">—</span></div><span id="session-info">Đang chờ thiết bị kết nối</span></div>
    </div>
    <div class="video-options"><button id="video-retry" hidden>Tải lại hình ảnh</button><button id="video-browser" hidden>Xem trong trình duyệt</button></div>
    <p id="panel-status" class="panel-status" role="status" hidden></p>
  </section>
</main>
<div id="session-menu" class="session-menu" role="dialog" aria-label="Tùy chọn phiên" hidden>
  <p class="menu-caption">Xem và can thiệp thủ công</p>
  <button id="focus-view" class="quiet" aria-label="Phóng to hình ảnh" aria-pressed="false" disabled>${icon("expand")}<span id="focus-label">Phóng to hình ảnh</span></button>
  <button id="take-over" class="quiet" disabled>${icon("mouse")}<span id="take-over-label">Tiếp quản thủ công</span></button>
  <div class="menu-audio"><button id="audio-play" class="quiet" aria-label="Phát âm thanh" aria-pressed="false" disabled>${icon("volume")}<span id="audio-label">Phát âm thanh</span></button><label class="audio-volume"><span>Âm lượng</span><input id="audio-volume" type="range" min="0" max="1" step="0.05" value="1" disabled></label></div>
  <button id="disconnect" class="quiet danger" disabled>${icon("power")}<span>Ngắt kết nối</span></button>
  <p class="menu-note">Ngắt kết nối sẽ kết thúc phiên từ xa hiện tại.</p>
</div>
<dialog id="unlock-dialog" class="unlock-dialog" aria-labelledby="unlock-title" aria-describedby="unlock-status">
  <div class="unlock-dialog-header"><h2 id="unlock-title">Mở khóa máy tính từ xa</h2><button id="unlock-close" type="button" aria-label="Đóng hộp mở khóa">×</button></div>
  <p id="unlock-status" role="status"></p>
  <form id="unlock-form">
    <label for="unlock-password">Mật khẩu khóa màn hình hệ thống</label><input id="unlock-password" type="password" autocomplete="off" maxlength="256" required autofocus>
    <div class="checks"><label><input id="unlock-remember" type="checkbox"><span>Lưu mật khẩu và cho phép tự mở khóa sau<small>Lưu riêng trong kho xác thực hệ thống trên máy này</small></span></label></div>
    <div class="unlock-buttons"><button id="unlock-submit" class="primary" type="submit">Mở khóa</button><button id="unlock-cancel" type="button">Hủy mở khóa</button></div>
  </form>
</dialog>
<div id="unlock-toast" class="unlock-toast" role="status" hidden><div><strong id="unlock-toast-title"></strong><p id="unlock-toast-message"></p></div><button id="unlock-toast-close" type="button" aria-label="Đóng thông báo mở khóa">×</button></div>`;
const element = <T extends HTMLElement = HTMLElement>(id: string) =>
  document.getElementById(id) as T;
const input = (id: string) => element<HTMLInputElement>(id);
const select = (id: string) => element<HTMLSelectElement>(id);
let invoke: Invoke;
let app: App | undefined;
let context: Data | undefined;
let status: Data = {};
let savedDevices: Data[] = [];
let controlBusy = false;
let unlockBusy = false;
let unlockRequestId: string | undefined;
let unlockSessionId: string | undefined;
let unlockState: string | undefined;
let unlockError = "";
let unlockToastTimer: ReturnType<typeof setTimeout> | undefined;
let disconnectedByUser = false;
let frameSize = "", frameRate = "";
let player: VideoPlayer | PreviewPlayer | undefined;
let usePreview = false;
let videoStarting = false, videoFailed = false, videoEpoch = 0;
let videoSession: string | undefined;
let videoGeneration: number | undefined;
let imageAt = 0;
let busy = false,
  polling = false,
  started = false;
const audioPlayer = new AudioPlayer();
const humanInput = new HumanInput(element("screen"), element<HTMLTextAreaElement>("remote-input"),
  (args) => invoke("input", args), showError);
function ownsInput() { return status.control_owner === "human" && status.human_view === context?.view_id; }
const messages: Record<string, string> = {
  AUDIO_INVALID: "Định dạng âm thanh từ xa không hợp lệ.",
  AUDIO_UNAVAILABLE: "Chưa nhận được âm thanh từ xa, hãy kiểm tra quyền ghi âm phía từ xa.",
  HUMAN_CONTROL_ACTIVE: "Người dùng đang thao tác máy từ xa, hãy trả quyền cho Agent trước.",
  STALE_OBSERVATION: "Hình ảnh đã thay đổi, hãy chờ hình mới rồi tiếp tục thao tác.",
  STALE_CONTROL: "Quyền điều khiển đã được bàn giao.",
  INPUT_QUEUE_FULL: "Nhập quá nhanh, đã nhả các phím. Hãy tiếp tục sau.",
  NATIVE_INPUT_UNSUPPORTED: "Hãy cập nhật CLI native để bật tiếp quản thủ công.",
  VIDEO_UNSUPPORTED: "Trình duyệt hiện tại không hỗ trợ định dạng video này, hãy dùng trình duyệt hỗ trợ WebCodecs.",
  VIDEO_UNAVAILABLE: "Kênh video tạm không khả dụng. Bạn có thể tải lại hình ảnh; nếu bảng nhúng bị giới hạn, hãy xem trong trình duyệt.",
  VIDEO_POLICY_BLOCKED: "Chính sách bảo mật của host đã chặn kết nối video trực tiếp.",
  VIDEO_INVALID: "Định dạng video đã thay đổi, hãy tải lại hình ảnh.",
  VIDEO_LIMIT: "Số bảng xem đã đạt giới hạn, hãy đóng các bảng khác.",
  AUTH_FAILED: "Sai mật khẩu truy cập, hãy nhập lại.",
  AUTH_REQUIRED: "Hãy nhập mật khẩu truy cập hoặc cấu hình thông tin xác thực qua setup trên máy này.",
  CONNECTION_FAILED: "Kết nối thất bại, hãy kiểm tra thiết bị có trực tuyến và cài đặt máy chủ.",
  CONNECTION_TIMEOUT: "Quá hạn chờ hình ảnh từ xa, hãy kiểm tra trạng thái máy từ xa rồi thử lại.",
  CONNECTION_LOST: "Kết nối từ xa đã bị ngắt. Hãy kiểm tra trạng thái máy từ xa và kết nối lại.",
  signaling_timeout: "Quá hạn kết nối signaling server, hãy kiểm tra mạng, proxy trên máy này và trạng thái máy chủ.",
  signaling_disconnected: "Kết nối signaling đã bị ngắt, hãy kiểm tra mạng, proxy trên máy này và trạng thái máy chủ.",
  signaling_login_failed: "Đăng nhập signaling server thất bại, hãy kiểm tra cấu hình máy chủ.",
  tls_certificate_error: "Xác minh chứng chỉ signaling server thất bại, hãy kiểm tra chứng chỉ và thời gian máy này.",
  cli_identity_not_supported: "Signaling server cần hỗ trợ đăng ký danh tính CLI, hãy cập nhật máy chủ.",
  join_request_failed: "Gửi yêu cầu kết nối thiết bị thất bại, hãy kiểm tra kết nối signaling.",
  incorrect_password: "Sai mật khẩu truy cập, hãy nhập lại.",
  device_not_found: "ID thiết bị không hợp lệ hoặc đã hết hạn, hãy xác nhận ID thiết bị.",
  device_offline_or_unavailable: "Tạm không thể kết nối thiết bị từ xa, hãy kiểm tra trạng thái trực tuyến của máy được điều khiển.",
  connection_timeout: "Quá hạn chờ kết nối từ xa, hãy kiểm tra máy được điều khiển và trạng thái mạng.",
  connection_failed: "Không thể thiết lập kết nối từ xa, hãy kiểm tra mạng hai bên và trạng thái relay server.",
  connection_lost: "Kết nối từ xa đã bị ngắt, hãy kiểm tra trạng thái mạng hai bên.",
  remote_disconnected: "Máy từ xa đã ngắt kết nối.",
  display_not_available: "Màn hình đã chọn hiện không khả dụng, hãy kiểm tra chỉ số màn hình.",
  media_initialization_failed: "Khởi tạo thành phần media cục bộ thất bại, hãy kiểm tra môi trường chạy CLI native.",
  control_event_overflow: "Hàng đợi sự kiện kết nối đã đầy, hãy kết nối lại sau.",
  SESSION_REPLACED: "Kết nối đã khôi phục, hãy dùng phiên đã cập nhật.",
  SESSION_LIMIT: "Đã có tám kết nối hoạt động, hãy ngắt một kết nối trước.",
  DEVICE_BUSY: "Thiết bị này đã có kết nối hoạt động.",
  CREDENTIAL_STORE_UNAVAILABLE: "Kho xác thực hệ thống không khả dụng, mật khẩu chưa được lưu.",
  CREDENTIAL_ROLLBACK_FAILED: "Lưu thông tin xác thực bị gián đoạn, hãy cấu hình lại mật khẩu qua setup trên máy này.",
  PAIRING_REQUIRED:
    "Hãy chạy lệnh ghép đôi setup --open-panel từ kết quả công cụ Agent trong terminal, rồi mở lại bảng.",
  PANEL_UNAUTHORIZED: "Quyền của bảng đã hết hiệu lực, hãy mở lại.",
  VIEW_UNAUTHORIZED: "Bảng này đã hết hiệu lực, hãy dùng bảng mới mở nhất.",
  CONTROL_DENIED: "Phiên hiện tại không có quyền điều khiển hoặc đã bị người dùng tạm dừng.",
  UNLOCK_PENDING: "Hãy hoàn tất hoặc hủy yêu cầu mở khóa hiện tại trước.",
  UNLOCK_CREDENTIAL_REQUIRED: "Hãy nhập mật khẩu khóa màn hình hệ thống.",
  UNLOCK_EXPIRED: "Yêu cầu mở khóa đã hết hiệu lực, hãy để Agent quan sát lại màn hình khóa và gửi yêu cầu mới.",
  UNLOCK_NOT_CONFIRMED: "Trạng thái mở khóa hệ thống chưa được xác nhận, hãy kiểm tra hình ảnh từ xa.",
  NATIVE_UNLOCK_UNSUPPORTED: "Hãy cập nhật CLI native để bật mở khóa màn hình.",
  SAVE_DEVICE_REQUIRED: "Hãy lưu thiết bị trước khi lưu mật khẩu khóa màn hình.",
  SAVE_UNLOCK_REQUIRED: "Hãy chọn lưu mật khẩu khóa màn hình.",
  NATIVE_PACKAGE_MISSING:
    "Thiếu thành phần chạy CLI cho nền tảng tương ứng, hãy kiểm tra đã bật optional dependencies khi cài gói.",
  INVALID_ARGUMENT: "Hãy kiểm tra ID thiết bị, máy chủ và nội dung nhập.",
  DEVICE_ALIAS_CONFLICT: "Ghi chú này đã thuộc thiết bị khác, hãy dùng ghi chú khác.",
};
function text(id: string, value: string) {
  if (element(id).textContent !== value) element(id).textContent = value;
}
function platformName(platform?: string) {
  return ({ macos: "macOS", windows: "Windows", linux: "Linux", android: "Android", ios: "iOS" } as Record<string, string>)[platform ?? ""] ?? "Máy tính từ xa";
}
function resetMetrics() {
  humanInput.setFrame(undefined);
  imageAt = 0;
  frameSize = frameRate = "";
  text("resolution", "—");
  text("fps", "—");
  element<HTMLButtonElement>("focus-view").disabled = true;
}
function setFocus(focused: boolean) {
  root.classList.toggle("focus-mode", focused);
  const button = element<HTMLButtonElement>("focus-view");
  const label = focused ? "Khôi phục bố cục" : "Phóng to hình ảnh";
  text("focus-label", label);
  if (focused) setDetails(false);
  button.setAttribute("aria-pressed", String(focused));
  button.setAttribute("aria-label", label);
  button.title = label;
}
function setDetails(open: boolean) {
  const restoreFocus = !open && root.classList.contains("details-open") &&
    element("sidebar").contains(document.activeElement);
  root.classList.toggle("details-open", open);
  element("details-backdrop").hidden = !open;
  element("details-toggle").setAttribute("aria-expanded", String(open));
  text("details-label", open ? "Thu gọn chi tiết" : "Chi tiết phiên");
  if (open) { setSessionMenu(false, false); element("details-close").focus(); }
  else if (restoreFocus && !root.classList.contains("focus-mode")) element("details-toggle").focus();
}
function positionSessionMenu() {
  const menu = element("session-menu");
  const rect = element("more-toggle").getBoundingClientRect();
  menu.style.top = `${Math.max(8, Math.min(rect.bottom + 6, window.innerHeight - menu.offsetHeight - 8))}px`;
  menu.style.right = `${Math.max(8, window.innerWidth - rect.right)}px`;
}
function setSessionMenu(open: boolean, restoreFocus = true) {
  const menu = element("session-menu");
  const hadFocus = menu.contains(document.activeElement);
  menu.hidden = !open;
  element("more-toggle").setAttribute("aria-expanded", String(open));
  if (open) {
    setDetails(false);
    positionSessionMenu();
    menu.querySelector<HTMLButtonElement>("button:not(:disabled):not([hidden])")?.focus();
  } else if (restoreFocus && hadFocus) element("more-toggle").focus();
}
element("details-toggle").onclick = () => setDetails(!root.classList.contains("details-open"));
element("details-close").onclick = element("details-backdrop").onclick = () => setDetails(false);
element("more-toggle").onclick = () => setSessionMenu(element("session-menu").hidden);
element("focus-view").onclick = () => {
  setSessionMenu(false);
  setFocus(!root.classList.contains("focus-mode"));
};
document.addEventListener("pointerdown", (event) => {
  if (!element("session-menu").hidden && event.target instanceof Node &&
      !element("session-menu").contains(event.target) && !element("more-toggle").contains(event.target))
    setSessionMenu(false, false);
});
document.addEventListener("focusin", (event) => {
  if (!element("session-menu").hidden && event.target instanceof Node &&
      !element("session-menu").contains(event.target) && !element("more-toggle").contains(event.target))
    setSessionMenu(false, false);
});
window.addEventListener("resize", () => { if (!element("session-menu").hidden) positionSessionMenu(); });
window.addEventListener("keydown", (event) => {
  if (event.key !== "Escape" || element<HTMLDialogElement>("unlock-dialog").open) return;
  if (!element("session-menu").hidden) { event.preventDefault(); setSessionMenu(false); }
  else if (root.classList.contains("details-open")) { event.preventDefault(); setDetails(false); }
  else if (!ownsInput() && root.classList.contains("focus-mode")) setFocus(false);
});
function hidePassword() {
  input("password").type = "password";
  text("password-toggle", "Hiện");
  element("password-toggle").setAttribute("aria-label", "Hiện mật khẩu");
  element("password-toggle").setAttribute("aria-pressed", "false");
}
element("password-toggle").onclick = () => {
  if (input("password").type === "text") return hidePassword();
  input("password").type = "text";
  text("password-toggle", "Ẩn");
  element("password-toggle").setAttribute("aria-label", "Ẩn mật khẩu");
  element("password-toggle").setAttribute("aria-pressed", "true");
};
select("mode").onchange = () => {
  text("mode-description", select("mode").value === "control"
    ? "Agent có thể thao tác chuột và bàn phím, bạn có thể tạm dừng bất cứ lúc nào."
    : "Agent có thể xem màn hình, không thể thao tác chuột và bàn phím.");
};
function showError(error: unknown) {
  const code = error instanceof Error ? error.message : "INTERNAL_ERROR";
  element("error").textContent =
    messages[code] ??
    `Thao tác chưa hoàn tất (${/^[A-Z_]+$/.test(code) ? code : "CONNECTION_ERROR"})`;
  element("error").hidden = false;
}
function check(result: Data): Data {
  if (result.error) {
    const error = typeof result.error === "string"
      ? { code: "CONNECTION_FAILED", reason: result.error } : result.error;
    throw new Error(Object.hasOwn(messages, error.reason)
      ? error.reason : error.code ?? "INTERNAL_ERROR");
  }
  return result;
}
async function devices() {
  const result = await invoke("devices");
  if (!started && result.defaults) {
    input("server").value = result.defaults.server;
    input("port").value = String(result.defaults.port);
  }
  const chosen = select("device").value || context?.device_alias || "";
  select("device").replaceChildren(new Option("Kết nối thiết bị mới", ""));
  savedDevices = result.devices ?? [];
  for (const d of savedDevices)
    select("device").add(
      new Option(`${d.device_alias} · ${d.device_id}`, d.device_alias),
    );
  select("device").value = [...select("device").options].some(
    (o) => o.value === chosen,
  )
    ? chosen
    : "";
  deviceChanged();
}
function deviceChanged() {
  const saved = !!select("device").value;
  element("new-device").hidden = saved;
  element("new-device-label").hidden = saved;
  input("server").disabled = input("port").disabled = saved;
  input("password").placeholder = saved
    ? "Để trống để dùng mật khẩu đã lưu"
    : "Nhập mật khẩu truy cập từ xa";
}
select("device").onchange = deviceChanged;
input("save").onchange = () => {
  for (const name of ["remember", "remember-unlock", "authorize"]) {
    input(name).disabled = !input("save").checked;
    if (!input("save").checked) input(name).checked = false;
  }
  savedUnlockChanged();
};
function savedUnlockChanged() {
  element("saved-unlock-field").hidden = !input("remember-unlock").checked;
  if (!input("remember-unlock").checked) input("saved-unlock-password").value = "";
}
input("remember-unlock").onchange = savedUnlockChanged;
function hideUnlockToast() {
  clearTimeout(unlockToastTimer);
  unlockToastTimer = undefined;
  element("unlock-toast").hidden = true;
}
function openUnlockDialog() {
  const dialog = element<HTMLDialogElement>("unlock-dialog");
  if (status.unlock?.state !== "awaiting_password" || dialog.open) return;
  hideUnlockToast();
  dialog.showModal();
}
element("unlock-open").onclick = openUnlockDialog;
element("unlock-close").onclick = () => element<HTMLDialogElement>("unlock-dialog").close();
element<HTMLDialogElement>("unlock-dialog").onclose = () => {
  if (element<HTMLDialogElement>("unlock-dialog").open) return;
  input("unlock-password").value = "";
  input("unlock-remember").checked = false;
};
element("unlock-toast-close").onclick = hideUnlockToast;
function renderUnlock(s: Data) {
  const request = s.unlock;
  const pending = request?.state === "awaiting_password";
  const visible = !!request && ["ready", "waiting_frame"].includes(s.state);
  const sessionChanged = s.session_id !== unlockSessionId;
  const changed = sessionChanged || request?.request_id !== unlockRequestId;
  const stateChanged = changed || request?.state !== unlockState;
  const finished = ["submitted", "verified", "cancelled", "failed"].includes(request?.state);
  const dialog = element<HTMLDialogElement>("unlock-dialog");
  if (changed || !pending || !visible) {
    input("unlock-password").value = "";
    input("unlock-remember").checked = false;
  }
  if (changed || finished) unlockError = "";
  if (changed || !visible) hideUnlockToast();
  if (dialog.open && (changed || !visible || finished)) dialog.close();
  unlockRequestId = request?.request_id;
  unlockSessionId = s.session_id;
  unlockState = request?.state;
  const device = savedDevices.find((d) => d.device_alias === s.device_alias);
  if (request?.credential_saved && device) device.unlock_credential_provider = "keyring";
  const provider = request?.credential_saved ? "keyring" : device?.unlock_credential_provider;
  text("session-unlock-credential", provider === "keyring" ? "Đã lưu · Tự mở khóa"
    : provider === "environment" ? "Biến môi trường · Tự mở khóa" : "Chờ cấu hình");
  element("session-unlock-row").hidden = !visible;
  const states: Record<string, string> = {
    preparing: "Đang chuẩn bị", awaiting_password: "Đang chờ mật khẩu", submitting: "Đang gửi",
    submitted: "Đã gửi", verifying: "Đang xác minh", verified: "Đã mở khóa", cancelled: "Đã hủy", failed: "Gửi thất bại",
  };
  text("session-unlock-state", states[request?.state] ?? "—");
  element("session-unlock-state").dataset.state = request?.state ?? "";
  element("unlock-open").hidden = !visible || !pending;
  element("attention").hidden = !visible || !pending;
  element<HTMLButtonElement>("unlock-open").disabled = unlockBusy;
  element("unlock-form").hidden = !pending;
  input("unlock-password").disabled = unlockBusy;
  input("unlock-remember").disabled = unlockBusy || !device;
  element<HTMLButtonElement>("unlock-submit").disabled = unlockBusy;
  element<HTMLButtonElement>("unlock-cancel").disabled = unlockBusy;
  text("unlock-status", unlockError || (unlockBusy || request?.state === "submitting"
    ? "Đang gửi yêu cầu mở khóa, vui lòng chờ." : "Hãy xác nhận ô mật khẩu hệ thống từ xa đã có tiêu điểm, rồi nhập mật khẩu khóa màn hình."));
  if (visible && pending && stateChanged) openUnlockDialog();
  if (visible && finished) {
    text("unlock-toast-title", request.state === "verified" ? "Hệ thống đã mở khóa" : request.state === "submitted" ? "Đã gửi yêu cầu mở khóa"
      : request.state === "cancelled" ? "Đã hủy mở khóa" : "Thao tác mở khóa chưa hoàn tất");
    let message = ["submitted", "verified"].includes(request.state) ? "Agent sẽ kiểm tra hình ảnh và tiếp tục tác vụ." : "";
    if (request.credential_saved) message = "Đã lưu mật khẩu khóa màn hình, Agent sẽ kiểm tra hình ảnh và tiếp tục tác vụ.";
    if (request.error) message = messages[request.error.code] ?? "Hãy kiểm tra hình ảnh từ xa rồi yêu cầu mở khóa lại.";
    if (request.persistence_error) message = request.persistence_error === "UNLOCK_NOT_CONFIRMED"
      ? "Mật khẩu khóa màn hình cần được lưu sau khi xác nhận mở khóa, hãy kiểm tra hình ảnh từ xa." : "Lưu mật khẩu thất bại, bạn có thể cấu hình lại qua setup trên máy này.";
    text("unlock-toast-message", message);
    element("unlock-toast-message").hidden = !message;
    element("unlock-toast").dataset.state = request.state;
    if (stateChanged && !sessionChanged) {
      element("unlock-toast").hidden = false;
      clearTimeout(unlockToastTimer);
      unlockToastTimer = setTimeout(hideUnlockToast, 6000);
    }
  }
}
element<HTMLFormElement>("unlock-form").onsubmit = async (event) => {
  event.preventDefault();
  if (unlockBusy || !unlockRequestId) return;
  const args: Data = { request_id: unlockRequestId, password: input("unlock-password").value,
    remember_password: input("unlock-remember").checked };
  input("unlock-password").value = "";
  unlockError = "";
  unlockBusy = true;
  renderUnlock(status);
  try {
    const result = await invoke("unlock", args);
    renderStatus({ ...status, unlock: result.unlock });
    await devices();
  } catch (error) {
    unlockError = messages[error instanceof Error ? error.message : ""] ?? "Tạm không thể gửi yêu cầu mở khóa, hãy kiểm tra trạng thái kết nối.";
  }
  finally { args.password = undefined; unlockBusy = false; renderUnlock(status); }
};
element("unlock-cancel").onclick = async () => {
  if (!unlockRequestId || unlockBusy) return;
  input("unlock-password").value = "";
  try {
    const result = await invoke("unlock", { request_id: unlockRequestId, cancel: true });
    renderStatus({ ...status, unlock: result.unlock });
  } catch (error) {
    unlockError = messages[error instanceof Error ? error.message : ""] ?? "Tạm không thể gửi yêu cầu hủy, hãy kiểm tra trạng thái kết nối.";
    renderUnlock(status);
  }
};
function renderStatus(s: Data) {
  const connected = s.state === "ready" || s.state === "waiting_frame";
  const hasSession = connected || s.state === "reconnecting";
  const lost = ["closed", "failed"].includes(s.state);
  if (lost && !disconnectedByUser && ["ready", "waiting_frame"].includes(status.state)) showError(new Error("CONNECTION_LOST"));
  const changedSession = s.session_id !== status.session_id;
  status = s;
  element<HTMLButtonElement>("audio-play").disabled = !connected || usePreview;
  input("audio-volume").disabled = !connected || usePreview;
  const audioLabel = audioPlayer.enabled ? "Tắt tiếng" : "Phát âm thanh";
  text("audio-label", audioLabel);
  element("audio-play").setAttribute("aria-label", audioLabel);
  element("audio-play").setAttribute("aria-pressed", String(audioPlayer.enabled));
  element("audio-play").title = audioLabel;
  if (!connected || changedSession) audioPlayer.flush();
  humanInput.update(connected && ownsInput() ? s.human_input_epoch : undefined);
  text("take-over-label", ownsInput() ? "Trả quyền cho Agent" : "Tiếp quản thủ công");
  element("take-over").setAttribute("aria-pressed", String(ownsInput()));
  element<HTMLButtonElement>("take-over").disabled = !connected || !s.control_allowed ||
    s.viewer_input_protocol !== "events/1" || controlBusy || (!imageAt && !ownsInput());
  element("take-over").hidden = !s.control_allowed || ownsInput();
  element("return-agent").hidden = !connected || !ownsInput();
  element<HTMLButtonElement>("return-agent").disabled = element<HTMLButtonElement>("take-over").disabled;
  renderUnlock(s);
  const connecting = busy || s.state === "waiting_frame" || s.state === "reconnecting";
  root.dataset.session = String(hasSession);
  root.dataset.connected = String(connected);
  root.dataset.connecting = String(connecting);
  root.dataset.controlling = String(connected && s.control === true && s.control_allowed === true && !s.human_paused);
  element("connection").dataset.state = connected ? "online" : connecting ? "connecting" : lost && !disconnectedByUser ? "offline" : "idle";
  text("connection-label", connected ? "Đã kết nối" : connecting ? "Đang kết nối" : lost ? "Chưa kết nối" : "Đang chờ kết nối");
  element("connect-form").hidden = connected || s.state === "reconnecting";
  element("session-card").hidden = !connected && s.state !== "reconnecting";
  text("sidebar-title", hasSession ? "Chi tiết phiên" : "Cấp quyền kết nối cho Agent");
  text("device-name", hasSession ? s.device_alias || "Máy tính từ xa" : "Hình ảnh từ xa");
  if (!hasSession || changedSession) { setDetails(false); setSessionMenu(false, false); }
  // Native control state describes permissions/ownership, not task execution.
  const agentState = s.state === "reconnecting" ? "recovering" : !connected ? "idle"
    : s.control_owner === "human" ? "human" : s.human_paused ? "paused"
    : s.unlock?.state === "awaiting_password" ? "unlock"
    : !s.control_allowed ? "observe" : s.control ? "enabled" : "idle";
  const agentLabel = s.state === "reconnecting" ? "Đang khôi phục kết nối" : !connected ? "Đang chờ cấp quyền"
    : agentState === "human" ? ownsInput() ? "Đang tiếp quản thủ công · Agent đã nhường quyền điều khiển" : "Người dùng đang thao tác trong bảng khác"
    : agentState === "paused" ? "Điều khiển của Agent đã tạm dừng" : agentState === "unlock" ? "Agent đang chờ mở khóa hệ thống"
    : agentState === "observe" ? "Agent chỉ xem" : agentState === "enabled" ? "Đã bật điều khiển của Agent" : "Chưa bật điều khiển của Agent";
  text("agent-state", agentLabel);
  element("agent-state").dataset.state = agentState;
  const platform = platformName(s.platform);
  text("session-info", connected ? `${platform} · Màn hình ${(s.display ?? 0) + 1}` : "Đang chờ thiết bị kết nối");
  text("session-device", s.device_alias || "Máy tính từ xa");
  text("session-platform", platform);
  const device = savedDevices.find((d) => d.device_alias === s.device_alias);
  element("session-device-id-row").hidden = !device;
  text("session-device-id", device ? String(device.device_id).replace(/(\d{3})(?=\d)/g, "$1 ") : "");
  text("session-access", s.control_allowed ? "Xem và thao tác" : "Chỉ xem");
  text("session-display", `Màn hình ${(s.display ?? 0) + 1}`);
  text("session-agent", agentLabel);
  input("recovery").checked = s.reconnect_enabled === true;
  input("recovery").disabled = !s.session_id;
  element("session-agent").dataset.paused = String(!!s.human_paused);
  element<HTMLButtonElement>("pause").disabled = !connected || !s.control_allowed || controlBusy || s.control_owner === "human";
  element("pause").hidden = !connected || !s.control_allowed || s.control_owner === "human";
  element("pause").classList.toggle("soft", !!s.human_paused);
  const pauseLabel = s.human_paused ? "Tiếp tục Agent" : "Tạm dừng Agent";
  text("pause-label", pauseLabel);
  element("pause").title = s.human_paused ? "Tiếp tục điều khiển của Agent" : "Tạm dừng điều khiển của Agent";
  element<HTMLButtonElement>("disconnect").disabled = !s.session_id || controlBusy || busy;
  element<HTMLButtonElement>("connect").disabled = busy || connected || !invoke;
  element<HTMLFieldSetElement>("connection-fields").disabled = busy;
  element("connect").setAttribute("aria-busy", String(busy));
  text("connect-label", busy ? "Đang kết nối…" : "Cấp quyền và kết nối");
  if (!connected) {
    element("frame").hidden = true;
    element("empty").hidden = false;
    element("stale").hidden = true;
    resetMetrics();
    if (root.classList.contains("focus-mode")) setFocus(false);
  } else if (changedSession) {
    resetMetrics();
    element("frame").hidden = true;
    element("empty").hidden = false;
  }
  if (!imageAt && !videoFailed) {
    text("empty-title", connecting || connected ? "Đang thiết lập kết nối từ xa" : lost ? "Phiên đã kết thúc" : "Chưa kết nối thiết bị");
    text("empty-description", connecting || connected ? "Đang chờ hình ảnh từ xa, vui lòng chờ." : lost ? "Hãy kết nối lại thiết bị." : "Chọn thiết bị và nhập mật khẩu truy cập.");
    text("video-state", connecting || connected ? "Đang chờ hình ảnh" : "Chờ kết nối");
    element("video-state").dataset.state = "idle";
  }
}
async function connect() {
  if (!invoke || busy || ["ready", "waiting_frame"].includes(status.state)) return;
  element("error").hidden = true;
  disconnectedByUser = false;
  busy = true;
  renderStatus(status);
  const saved = select("device").value;
  let args: Data = {
    mode: select("mode").value,
    display_index: Number(input("display").value),
    save_device: input("save").checked,
    remember_password: input("remember").checked,
    remember_unlock_password: input("remember-unlock").checked,
    persistent_authorization: input("authorize").checked,
    ...(saved
      ? { device_alias: saved }
      : {
          device_id: input("device-id").value.replace(/\s/g, ""),
          server: input("server").value,
          port: Number(input("port").value),
          ...(input("label").value ? { label: input("label").value } : {}),
        }),
  };
  if (!saved || input("password").value)
    args.password = input("password").value;
  if (input("remember-unlock").checked) args.unlock_password = input("saved-unlock-password").value;
  input("password").value = "";
  input("saved-unlock-password").value = "";
  hidePassword();
  try {
    const result = await invoke("connect", args);
    args = {};
    renderStatus(result);
    if (result.persistence_error)
      showError(new Error(result.persistence_error));
    await devices();
  } catch (error) {
    args = {};
    showError(error);
  } finally {
    busy = false;
    renderStatus(status);
  }
}
element("connect").onclick = () => {
  void connect();
};
element("connect-form").addEventListener("keydown", (event) => {
  if (event.key === "Enter" && event.target instanceof HTMLInputElement) {
    event.preventDefault();
    void connect();
  }
});
input("recovery").onchange = async () => {
  input("recovery").disabled = true;
  try {
    await invoke("recovery", { enabled: input("recovery").checked });
    renderStatus(await invoke("status"));
  } catch (error) { showError(error); renderStatus(status); }
};
element("audio-play").onclick = async () => {
  const enabled = !audioPlayer.enabled;
  element<HTMLButtonElement>("audio-play").disabled = true;
  try {
    await audioPlayer.enable(enabled);
    audioPlayer.volume(Number(input("audio-volume").value));
    await invoke("audio", { enabled });
  } catch (error) { await audioPlayer.enable(false); showError(error); }
  finally { renderStatus(status); }
};
input("audio-volume").oninput = () => audioPlayer.volume(Number(input("audio-volume").value));
element("take-over").onclick = element("return-agent").onclick = async () => {
  if (controlBusy) return;
  setSessionMenu(false);
  controlBusy = true;
  humanInput.release();
  renderStatus(status);
  try {
    await invoke("control", { operation: ownsInput() ? "resume" : "take_over" });
    renderStatus(await invoke("status"));
  } catch (error) { showError(error); }
  finally { controlBusy = false; renderStatus(status); }
};
element("pause").onclick = async () => {
  if (controlBusy) return;
  controlBusy = true;
  renderStatus(status);
  try {
    await invoke("control", {
      operation: status.human_paused ? "resume" : "pause",
    });
    renderStatus(await invoke("status"));
  } catch (e) {
    showError(e);
  } finally {
    controlBusy = false;
    renderStatus(status);
  }
};
element("disconnect").onclick = async () => {
  if (controlBusy) return;
  setSessionMenu(false);
  controlBusy = true;
  disconnectedByUser = true;
  renderStatus(status);
  try {
    await closePreview();
    await invoke("control", { operation: "disconnect" });
    element("error").hidden = true;
    resetMetrics();
    renderStatus(await invoke("status"));
  } catch (e) {
    disconnectedByUser = false;
    showError(e);
  } finally {
    controlBusy = false;
    renderStatus(status);
  }
};
async function closePreview() {
  audioPlayer.flush();
  ++videoEpoch;
  player?.stop();
  player = undefined;
  videoSession = undefined;
  videoGeneration = undefined;
  videoStarting = videoFailed = false;
  element("video-retry").hidden = true;
  element("video-browser").hidden = true;
  element("panel-status").hidden = true;
}
async function startPreview() {
  if (player || videoStarting || videoFailed || document.hidden) return;
  const epoch = ++videoEpoch;
  videoSession = status.session_id;
  videoGeneration = status.generation;
  videoStarting = true;
  const failed = (error: Error) => {
    if (epoch !== videoEpoch) return;
    if (CROSSDESK_APPS && !usePreview &&
        ["VIDEO_POLICY_BLOCKED", "VIDEO_UNAVAILABLE", "VIDEO_UNSUPPORTED"].includes(error.message)) {
      player?.stop();
      player = undefined;
      usePreview = true;
      videoStarting = false;
      void startPreview();
      return;
    }
    videoFailed = true;
    showError(error);
    element("video-retry").hidden = false;
    element("video-browser").hidden = !CROSSDESK_APPS;
    element("stale").hidden = !imageAt;
    text("video-state", "Hình ảnh không khả dụng");
    element("video-state").dataset.state = "stale";
    if (!imageAt) {
      text("empty-title", "Tạm không thể hiển thị hình ảnh");
      text("empty-description", "Phiên vẫn được giữ. Hãy thử tải lại hình ảnh.");
    }
  };
  const presented = (info: FrameInfo) => {
    if (epoch !== videoEpoch) return;
    humanInput.setFrame({ ...info, ...(info.sourceSize ?? {}) });
    imageAt = Date.now();
    element("frame").hidden = false;
    element("empty").hidden = true;
    element("stale").hidden = true;
    const size = `${info.width} × ${info.height}`;
    const rate = info.fps > 0 ? `${info.fps.toFixed(1)} FPS` : "Đang đo";
    if (size !== frameSize) { frameSize = size; text("resolution", size); }
    if (rate !== frameRate) { frameRate = rate; text("fps", rate); }
    element<HTMLButtonElement>("focus-view").disabled = false;
    text("video-state", usePreview ? "Xem trước" : "Trực tiếp");
    element("video-state").dataset.state = "live";
  };
  try {
    if (usePreview) {
      audioPlayer.flush();
      element<HTMLButtonElement>("audio-play").disabled = true;
      input("audio-volume").disabled = true;
      element("error").hidden = true;
      text("panel-status", "Video trực tiếp không khả dụng, đã chuyển sang xem trước nhúng: tối đa 1 khung/giây, không âm thanh. Bạn có thể dùng bảng trình duyệt nếu cần hình ảnh mượt.");
      element("panel-status").hidden = false;
      const preview = new PreviewPlayer(element<HTMLCanvasElement>("frame"), status.generation,
        (args) => invoke("preview", args), presented, failed);
      player = preview;
      preview.start();
      return;
    }
    const grant = await invoke("video_open");
    if (epoch !== videoEpoch || document.hidden) return;
    const video = new VideoPlayer(element<HTMLCanvasElement>("frame"), presented,
      failed, element<HTMLCanvasElement>("agent-cursor"), (bytes) => audioPlayer.accept(bytes));
    player = video;
    video.start(grant.url, grant.ticket, grant.protocol, grant.decoder);
  } catch (error) {
    failed(error instanceof Error ? error : new Error("VIDEO_UNAVAILABLE"));
  } finally {
    if (epoch === videoEpoch) videoStarting = false;
  }
}
element("video-retry").onclick = async () => {
  await closePreview();
  usePreview = false;
  element("error").hidden = true;
  await startPreview();
};
element("video-browser").onclick = async () => {
  try {
    const result = await app!.callServerTool({name: "crossdesk_open_browser", arguments: {view_id: context!.view_id}});
    if (result.isError) throw new Error("VIDEO_UNAVAILABLE");
    started = false;
    await closePreview();
    element("error").hidden = true;
    for (const name of ["connect", "pause", "disconnect"])
      element<HTMLButtonElement>(name).disabled = true;
    element("panel-status").hidden = false;
    element("panel-status").textContent = "Đã mở bảng xem trong trình duyệt.";
  } catch (error) { showError(error); }
};
async function poll() {
  if (polling || !invoke || !started || document.hidden || busy || controlBusy) return;
  polling = true;
  try {
    renderStatus(await invoke("status"));
    if (videoSession && (videoSession !== status.session_id ||
        videoGeneration !== status.generation)) {
      if (videoFailed) element("error").hidden = true;
      await closePreview();
    }
    if (status.state !== "ready" || document.hidden) {
      await closePreview();
      return;
    }
    await startPreview();
    const stale = !!imageAt && Date.now() - imageAt > 2000;
    element("stale").hidden = !stale;
    if (stale) {
      text("video-state", "Đang chờ hình ảnh");
      element("video-state").dataset.state = "stale";
      text("fps", "—");
      frameRate = "";
    }
  } catch (error) { showError(error); }
  finally { polling = false; }
}
document.addEventListener("visibilitychange", () => {
  if (document.hidden) void closePreview();
  else void poll();
});
window.addEventListener("pagehide", () => {
  element<HTMLDialogElement>("unlock-dialog").close();
  hideUnlockToast();
  input("unlock-password").value = "";
  input("saved-unlock-password").value = "";
  void closePreview();
});
setInterval(() => {
  void poll();
}, 1000);
async function ready(next: Data) {
  if (
    started &&
    context?.view_id === next.view_id &&
    context?.view_generation === next.view_generation
  )
    return;
  if (started) await closePreview();
  context = next;
  element("panel-status").hidden = true;
  started = false;
  await invoke("ready");
  await devices();
  started = true;
  element<HTMLButtonElement>("connect").disabled = false;
  await poll();
}
async function start() {
  if (CROSSDESK_APPS) {
    const { App } = await import("@modelcontextprotocol/ext-apps");
    app = new App({ name: "CrossDesk MCP", version: "0.1.0" }, {});
    const names: Record<string, string> = {
      ready: "crossdesk_ui_ready",
      devices: "crossdesk_ui_list_devices",
      status: "crossdesk_ui_get_session",
      connect: "crossdesk_ui_connect",
      video_open: "crossdesk_video_open",
      preview: "crossdesk_ui_preview",
      control: "crossdesk_viewer_control",
      unlock: "crossdesk_ui_unlock",
      recovery: "crossdesk_ui_recovery",
      input: "crossdesk_ui_input",
      audio: "crossdesk_ui_audio",
    };
    let connected = false,
      initial: Data | undefined;
    invoke = async (name, args = {}) => {
      const result = await app!.callServerTool({
        name: names[name]!,
        arguments: {
          ...args,
          view_id: context!.view_id,
          view_generation: context!.view_generation,
          ui_token: context!.ui_token,
        },
      });
      const data =
        result.structuredContent ??
        JSON.parse((result.content?.[0] as Data)?.text ?? "{}");
      check(data);
      return name === "preview" ? (result._meta?.["crossdesk/preview"] as Data) ?? data : data;
    };
    app.ontoolresult = (result) => {
      const meta = result._meta?.["crossdesk/ui"] as Data | undefined;
      if (!meta) return;
      initial = { ...(result.structuredContent ?? {}), ...meta };
      if (connected) void ready(initial).catch(showError);
    };
    app.onteardown = async () => {
      await closePreview();
      return {};
    };
    await app.connect();
    connected = true;
    if (initial) await ready(initial);
  } else {
    let csrf = "";
    const request = async (path: string, body?: Data) =>
      check(
        await (
          await fetch("/api/" + path, {
            method: body ? "POST" : "GET",
            credentials: "same-origin",
            headers: body
              ? { "Content-Type": "application/json", "X-CrossDesk-CSRF": csrf }
              : {},
            body: body ? JSON.stringify(body) : undefined,
            cache: "no-store",
          })
        ).json(),
      );
    const bootstrap = new URLSearchParams(location.hash.slice(1)).get(
      "bootstrap",
    );
    history.replaceState(null, "", location.pathname);
    const result = bootstrap
      ? await request("bootstrap", { token: bootstrap })
      : await request("context");
    csrf = result.csrf;
    invoke = (name, args = {}) =>
      request(name, { ...args, view_id: context!.view_id });
    const previous = sessionStorage.getItem("crossdesk-view");
    const selected = bootstrap ? result : result.views.find((view: Data) => view.view_id === previous) ?? result.views.at(-1);
    await ready(selected);
    sessionStorage.setItem("crossdesk-view", selected.view_id);
  }
}
void start().catch(showError);
