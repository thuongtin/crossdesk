const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const ts = require(process.argv[2]), root = process.argv[3];
const cache = new Map();

function load(name) {
  if (cache.has(name)) return cache.get(name);
  const exports = {};
  cache.set(name, exports);
  const compiled = ts.transpileModule(fs.readFileSync(path.join(root, name + '.ets'), 'utf8'), {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2020, experimentalDecorators: true },
    reportDiagnostics: true
  });
  assert.equal(compiled.diagnostics.filter(d => d.category === ts.DiagnosticCategory.Error).length, 0);
  vm.runInNewContext(compiled.outputText, {
    exports, console, Observed: value => value,
    require: name => ['Localization', 'LocalizationData', 'ControllerModels'].includes(name.slice(2))
      ? load(name.slice(2)) : {}
  });
  return exports;
}

const { tr, setLanguageProvider } = load('Localization');
const { fileTransferStatus } = load('ControllerModels');
const { MobileController } = load('MobileController');
setLanguageProvider(() => 'vi-VN');
assert.equal(fileTransferStatus(true, 'report.txt', 0.42), 'Gửi report.txt · 42%');
assert.equal(fileTransferStatus(false, 'report.txt', 1), `Nhận report.txt · ${tr('已完成')}`);
assert.equal(fileTransferStatus(true, '', 0), 'Gửi  · 0%');
const filename = '报告{0}.txt';
assert.equal(fileTransferStatus(false, filename, -1), tr('{0} 传输失败', [filename]));
assert(fileTransferStatus(true, filename, 0.5).includes(filename));

const controller = Object.create(MobileController.prototype);
controller.session = 7;
controller.connection = 11;
controller.state = { ready: true, connecting: false };
const shown = [], ended = [];
let stopped = 0;
controller.toast = text => shown.push(text);
controller.end = text => ended.push(text);
controller.stopSignal = () => ++stopped;
const feed = event => controller.event({ session: 7, connection: 11, ...event });
const native = fs.readFileSync(path.resolve(root, '../../../../../native/mobile_session.cpp'), 'utf8');
const reasons = [
  ['远程会话信令连接失败', 'Kết nối signaling phiên từ xa thất bại'],
  ['无法发起连接', 'Không thể bắt đầu kết nối'],
  ['服务器连接超时，请检查地址和网络', 'Kết nối máy chủ hết thời gian chờ, hãy kiểm tra địa chỉ và mạng'],
  ['连接超时，请确认设备在线', 'Kết nối hết thời gian chờ, hãy xác nhận thiết bị đang trực tuyến'],
  ['操作失败，请检查连接参数或重新连接', 'Thao tác thất bại, hãy kiểm tra tham số kết nối hoặc kết nối lại']
];
for (const [reason, expected] of reasons) {
  assert(native.includes(JSON.stringify(reason)));
  feed({ type: 'ended', reason });
  assert.equal(ended.at(-1), expected);
}
feed({ type: 'ended' });
assert.equal(ended.at(-1), '');
feed({ type: 'feedback', text: '已发送 Ctrl+Alt+Del' });
assert.equal(shown.at(-1), tr('已发送 Ctrl+Alt+Del'));
feed({ type: 'feedback', text: 'raw {0}' });
assert.equal(shown.at(-1), 'raw {0}');
feed({ type: 'feedback' });
assert.equal(shown.at(-1), '');
feed({ type: 'clipboard', text: '复制' });
assert.equal(controller.state.remoteClipboard, '复制');
feed({ type: 'file', sending: true, name: filename, progress: 1 });
assert.equal(controller.state.transfer, fileTransferStatus(true, filename, 1));

controller.state.connecting = true;
feed({ type: 'error', text: reasons[2][0] });
assert.equal(ended.at(-1), reasons[2][1]);
assert.equal(stopped, 1);
controller.state.connecting = false;
controller.state.ready = false;
feed({ type: 'error', text: reasons[4][0] });
assert.equal(controller.state.signalText, reasons[4][1]);
assert.equal(stopped, 2);
controller.state.ready = true;
feed({ type: 'error' });
assert.equal(shown.at(-1), tr('操作失败'));
const before = shown.length;
feed({ type: 'feedback', text: 'ignored', session: 8 });
assert.equal(shown.length, before);

setLanguageProvider(() => 'en-US');
assert.equal(fileTransferStatus(true, 'report.txt', 0.42), 'Send report.txt · 42%');
feed({ type: 'ended', reason: reasons[1][0] });
assert.equal(ended.at(-1), 'Unable to initiate connection');
console.log('PASS: localized native feedback/errors/end reasons, file progress/empty/error states, English override, stale events and unchanged clipboard/file-name data');
