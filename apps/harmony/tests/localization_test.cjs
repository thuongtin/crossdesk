const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const ts = require(process.argv[2]), root = process.argv[3];
const modules = {};
function load(name) {
  if (modules[name]) return modules[name];
  const exports = {};
  modules[name] = exports;
  vm.runInNewContext(ts.transpileModule(fs.readFileSync(path.join(root, name + '.ets'), 'utf8'), {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2020 }
  }).outputText, { exports, require: name => load(name.slice(2)) });
  return exports;
}
const { languageIndex, currentLanguage, setLanguageProvider, tr } = load('Localization');
for (const [tag, index] of [
  ['zh-CN', 0], ['en-US', 1], ['ru', 2], ['zh-Hant', 3], ['zh-TW', 3],
  ['zh-Hans-HK', 0], ['ja', 4], ['ko', 5], ['de', 6], ['fr', 7], ['es', 8],
  ['pt-PT', 9], ['vi', 10], ['VI_vn.UTF-8', 10], ['', 10], ['id-ID', 10]
]) assert.equal(languageIndex(tag), index, tag);
assert.equal(currentLanguage(), 10);
assert.equal(tr('连接'), 'Kết nối');
const translations = load('LocalizationData').translations;
const saved = translations['连接'][10];
translations['连接'][10] = '';
assert.equal(tr('连接'), '');
translations['连接'][10] = saved;
assert.equal(tr('unknown key'), 'unknown key');
setLanguageProvider(() => 'vi-VN');
assert.equal(tr('{0} 台设备', ['{1}']), '{1} thiết bị');
assert.equal(tr('{0} 台设备'), '{0} thiết bị');
setLanguageProvider(() => 'en-GB');
assert.equal(tr('连接'), 'Connect');
setLanguageProvider(() => 'unsupported');
assert.equal(tr('连接'), 'Kết nối');
setLanguageProvider(() => { throw Error('locale unavailable'); });
assert.equal(currentLanguage(), 10);
assert.equal(tr('连接'), 'Kết nối');
console.log('PASS: 11 locale mappings, Vietnamese default/empty/error fallback, English override and placeholders');
