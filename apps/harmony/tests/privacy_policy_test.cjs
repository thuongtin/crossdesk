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
const { selectPrivacyPolicy } = load('PrivacyPolicy');
const { setLanguageProvider } = load('Localization');
const bilingual = '## Tiếng Việt\nVietnamese\n### Details\nDetails\n## English\nEnglish';
const vi = 'Vietnamese\n### Details\nDetails';
for (const [tag, expected] of [
  ['vi', vi], ['vi-VN', vi], ['VI_vn.UTF-8', vi], ['', vi], ['id-ID', vi],
  ['en', 'English'], ['zh-CN', 'English'], ['zh-Hant', 'English'], ['ru', 'English'],
  ['ja', 'English'], ['ko', 'English'], ['de', 'English'], ['fr', 'English'],
  ['es', 'English'], ['pt-BR', 'English'], ['tr', 'English'], ['he', 'English'], ['iw', 'English']
]) {
  setLanguageProvider(() => tag);
  assert.equal(selectPrivacyPolicy(bilingual), expected, tag);
}
setLanguageProvider(() => { throw Error('locale unavailable'); });
assert.equal(selectPrivacyPolicy(bilingual), vi);
assert.equal(selectPrivacyPolicy(bilingual, -1), vi);
assert.equal(selectPrivacyPolicy('## 中文\r\nLegacy\r\n## English\r\nEnglish', 12), 'Legacy');
assert.equal(selectPrivacyPolicy('## 中文\nLegacy\n## English\nEnglish', 0), 'English');
for (const index of [1, 12]) {
  assert.equal(selectPrivacyPolicy('## English\nEnglish', index), 'English');
  assert.equal(selectPrivacyPolicy('## Tiếng Việt\nVietnamese', index), 'Vietnamese');
  assert.equal(selectPrivacyPolicy('## Tiếng Việt\n## English\nEnglish', index), 'English');
  assert.equal(selectPrivacyPolicy('## English\n## Tiếng Việt\nVietnamese', index), 'Vietnamese');
  assert.equal(selectPrivacyPolicy('## English\nEnglish\n## Tiếng Việt\nVietnamese', index), index === 12 ? 'Vietnamese' : 'English');
  assert.equal(selectPrivacyPolicy('Plain text', index), 'Plain text');
  assert.equal(selectPrivacyPolicy('', index), '');
  assert.equal(selectPrivacyPolicy(' \r\n ', index), '');
}
const policy = fs.readFileSync(path.join(__dirname, '../../../PRIVACY.md'), 'utf8');
for (const [tag, marker] of [['vi-VN', 'Cập nhật ngày:'], ['', 'Cập nhật ngày:'], ['id-ID', 'Cập nhật ngày:'], ['en-US', 'Updated:'], ['zh-CN', 'Updated:']]) {
  setLanguageProvider(() => tag);
  const selected = selectPrivacyPolicy(policy);
  assert(selected.includes(marker));
  assert(!selected.includes('## English') && !selected.includes('## Tiếng Việt'));
  console.log(`Policy reproduction: locale=${tag || '(default)'}, section=${marker === 'Updated:' ? 'English' : 'Vietnamese'}, bilingual=false`);
}
console.log('PASS: privacy policy locale/default/error fallback, legacy and independent sections, empty/missing sections, CRLF and bundled policy');
