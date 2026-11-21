import { currentLanguage } from './Localization';

function policySection(text: string, heading: string): string {
  const match = new RegExp('^## ' + heading + '[ \\t]*$', 'm').exec(text);
  if (!match) return '';
  const start = match.index + match[0].length;
  const end = text.indexOf('\n## ', start);
  return text.substring(start, end < 0 ? text.length : end).trim();
}

export function selectPrivacyPolicy(body: string, language: number = currentLanguage()): string {
  const text = body.replace(/\r\n/g, '\n');
  const vietnamese = policySection(text, 'Tiếng Việt') || policySection(text, '中文');
  const english = policySection(text, 'English');
  const useEnglish = language >= 0 && language < 10;
  return (useEnglish ? english || vietnamese : vietnamese || english) || text.trim();
}
