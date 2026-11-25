#!/usr/bin/env node
import { parseArgs } from "node:util";
import { VERSION } from "./version.js";
import { createInterface } from "node:readline/promises";
import { Writable } from "node:stream";
import {
  serveStdio,
  StdioServerTransport,
} from "@modelcontextprotocol/server/stdio";
import { DeviceStore, type Device } from "./store.js";
import { Sessions } from "./sessions.js";
import { BrowserPanel, pairBrowser, openURL } from "./browser.js";
import { createServer } from "./server.js";
import { TerminalPanel } from "./terminal.js";
import { terminalView } from "./terminal-view.js";
import {
  panel,
  browserLaunch,
  parse,
  Failure,
  failure,
  uiConnect,
  videoOptions,
  unlockPassword,
  type Panel,
} from "./common.js";

async function setup(store: DeviceStore, view?: string) {
  if (!process.stdin.isTTY || !process.stdout.isTTY)
    throw new Failure("SETUP_REQUIRES_TERMINAL");
  if (view) {
    const url = await pairBrowser(store.directory, view);
    if (!url) throw new Failure("PANEL_NOT_FOUND");
    if (await openURL(url)) process.stdout.write("Đã mở trang ghép đôi trình duyệt.\n");
    else
      process.stdout.write(
        `Hãy mở liên kết dùng một lần dưới đây trong trình duyệt trên máy này (hiệu lực 60 giây, không gửi vào chat):\n${url}\n`,
      );
    return;
  }
  let hidden = false;
  const output = new Writable({
    write(chunk, _encoding, done) {
      if (!hidden) process.stdout.write(chunk);
      done();
    },
  });
  const rl = createInterface({
    input: process.stdin,
    output,
    terminal: true,
    historySize: 0,
  });
  const question = async (label: string) => (await rl.question(label)).trim();
  try {
    const config = await store.config();
    process.stdout.write(
      "CrossDesk MCP · Cấu hình thiết bị cục bộ\nMật khẩu chỉ lưu trong kho thông tin xác thực hệ thống, không ghi vào cấu hình thiết bị.\n",
    );
    const name = await question("Bí danh thiết bị:");
    const previous = config.devices.find((d) => d.alias === name);
    const device = previous?.device_id ?? (await question("ID thiết bị 9 chữ số:"));
    const server =
      previous?.server ??
      ((await question(`Máy chủ [${config.server}]: `)) || config.server);
    const port =
      previous?.port ??
      Number((await question(`Cổng [${config.port}]: `)) || config.port);
    const permission = await question(
      "Cho phép tự kết nối sau? [n / observe / control]:",
    );
    const environment = await question(
      "Dùng biến môi trường mật khẩu hiện có (để trống để dùng kho xác thực hệ thống):",
    );
    let password: string | undefined;
    if (!environment) {
      hidden = true;
      process.stdout.write("Mật khẩu truy cập (nhập ẩn, có thể để trống):");
      try {
        password = await rl.question("");
      } finally {
        hidden = false;
        process.stdout.write("\n");
      }
    }
    parse(uiConnect, {
      device_id: device,
      label: name,
      server,
      port,
      password,
    });
    const d: Device = {
      alias: name,
      device_id: device,
      server,
      port,
      ...(permission === "observe" || permission === "control"
        ? { authorization: permission }
        : {}),
      ...(environment
        ? { credential: "environment", environment_variable: environment }
        : {}),
      ...(previous?.unlock_credential ? {
        unlock_credential: previous.unlock_credential,
        unlock_environment_variable: previous.unlock_environment_variable,
      } : {}),
    };
    let unlockSecret: string | undefined;
    if ((await question("Cấu hình mật khẩu khóa màn hình để Agent tự mở khóa sau này? [y/N]:")).toLowerCase() === "y") {
      const variable = await question("Tên biến môi trường mật khẩu khóa màn hình (để trống để dùng kho xác thực hệ thống):");
      if (variable) {
        d.unlock_credential = "environment";
        d.unlock_environment_variable = variable;
      } else {
        hidden = true;
        process.stdout.write("Mật khẩu khóa màn hình hệ thống (nhập ẩn):");
        try { unlockSecret = parse(unlockPassword, await rl.question("")); }
        finally { hidden = false; process.stdout.write("\n"); }
      }
    }
    await store.save(d, password, unlockSecret);
    password = undefined;
    unlockSecret = undefined;
    process.stdout.write(
      "Đã lưu. Agent có thể gọi crossdesk_list_devices; tự kết nối vẫn cần quyền và thông tin xác thực hợp lệ.\n",
    );
  } finally {
    hidden = false;
    rl.close();
  }
}
async function main() {
  const { values, positionals } = parseArgs({
    allowPositionals: true,
    strict: true,
    options: {
      stdio: { type: "boolean" },
      panel: { type: "string" },
      "browser-launch": { type: "string" },
      "video-path": { type: "string" },
      "video-decoder": { type: "string" },
      "config-dir": { type: "string" },
      "native-binary": { type: "string" },
      "open-panel": { type: "boolean" },
      view: { type: "string" },
      "cell-aspect": { type: "string" },
      help: { type: "boolean" },
      version: { type: "boolean" },
    },
  });
  if (values.version) {
    process.stdout.write(`${VERSION}\n`);
    return;
  }
  if (values.help || !positionals.length) {
    process.stdout.write(
      "crossdesk-mcp serve --stdio [--panel auto|apps|browser|terminal|off] [--browser-launch auto|host|system] [--video-path browser|minirtc] [--video-decoder auto|software] [--config-dir directory]\nDefaults: --panel=auto --browser-launch=auto (Codex/DeepSeek Harness: host sidebar browser; other clients: local environment/system browser)\nExplicit host sidebar: --panel=browser --browser-launch=host\nSystem browser: --panel=browser --browser-launch=system\ncrossdesk-mcp view --view VIEW_ID [--config-dir directory] [--cell-aspect 0.5]\ncrossdesk-mcp setup [--config-dir directory]\ncrossdesk-mcp setup --open-panel --view VIEW_ID [--config-dir directory]\nDevelopment: --native-binary /absolute/path/to/crossdesk-cli\n",
    );
    return;
  }
  if (positionals.length !== 1 || !["serve", "setup", "view"].includes(positionals[0]!))
    throw new Failure("INVALID_COMMAND");
  const store = new DeviceStore(
    values["config-dir"] ?? process.env.CROSSDESK_MCP_CONFIG_DIR,
  );
  await store.initialize();
  if (positionals[0] === "view") {
    if (!values.view) throw new Failure("VIEW_REQUIRED");
    await terminalView(store.directory, values.view, Number(values["cell-aspect"] ?? 0.5));
    return;
  }
  if (positionals[0] === "setup") {
    if (values["open-panel"] && !values.view)
      throw new Failure("VIEW_REQUIRED");
    await setup(store, values["open-panel"] ? values.view : undefined);
    return;
  }
  if (!values.stdio) throw new Failure("STDIO_REQUIRED");
  const config = await store.config();
  const policy: Panel = parse(
    panel,
    values.panel ?? config.panel,
  );
  const launch = parse(browserLaunch, values["browser-launch"] ?? "auto");
  // Other explicit panel policies do not use the browser launch default.
  // auto resolves per client when a browser panel is opened.
  if (values["browser-launch"] === "host" && !["auto", "browser"].includes(policy))
    throw new Failure("BROWSER_LAUNCH_REQUIRES_BROWSER");
  const sessions = new Sessions(
    store,
    policy,
    values["native-binary"] ?? process.env.CROSSDESK_NATIVE_BINARY,
    parse(videoOptions, {
      decode_path: values["video-path"] ?? config.video.decode_path,
      decoder: values["video-decoder"] ?? config.video.decoder,
    }),
  );
  const browser = new BrowserPanel(sessions, launch);
  const terminal = new TerminalPanel(sessions);
  const handle = serveStdio(() => createServer(sessions, browser, terminal), {
    transport: new StdioServerTransport(),
    onerror: () => process.stderr.write("crossdesk-mcp: protocol error\n"),
  });
  let closing = false;
  const close = async () => {
    if (closing) return;
    closing = true;
    await terminal.close();
    await browser.close();
    await sessions.close();
    await handle.close();
  };
  process.stdin.once("end", () => {
    void close();
  });
  for (const signal of ["SIGINT", "SIGTERM", "SIGHUP"] as const)
    process.once(signal, () => {
      void close().finally(() => {
        process.exitCode = 0;
      });
    });
  process.once("uncaughtException", () => {
    process.stderr.write("crossdesk-mcp: INTERNAL_ERROR\n");
    void close().finally(() => {
      process.exitCode = 1;
    });
  });
}
main().catch((error) => {
  const result = failure(error);
  process.stderr.write(`crossdesk-mcp: ${result.code}${result.reason ? ` (${result.reason})` : ""}\n`);
  process.exitCode = 1;
});
