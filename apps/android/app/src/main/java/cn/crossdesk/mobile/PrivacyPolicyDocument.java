package cn.crossdesk.mobile;

import android.content.Context;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import java.io.IOException;
import java.util.Arrays;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static cn.crossdesk.mobile.MobileUi.*;

/** The same bundled policy as iOS, readable before any network consent. */
final class PrivacyPolicyDocument {
    final boolean available;
    final String text;

    PrivacyPolicyDocument(Context context){this(load(context),L10n.language());}
    PrivacyPolicyDocument(String document){this(document,"vi");}
    PrivacyPolicyDocument(String document,String language){
        available=document!=null&&!document.trim().isEmpty();
        String value=available?document:L10n.tr("隐私政策暂时无法读取，请拒绝联网并重新安装应用。");
        text=selectText(value,language);
    }
    static String selectText(String document,String language){
        String value=document==null?"":document.replace("\r\n","\n");
        String vietnamese=section(value,"Tiếng Việt");
        if(vietnamese.isEmpty())vietnamese=section(value,"中文");
        String english=section(value,"English");
        String base=language==null?"":language.toLowerCase(Locale.ROOT).replace('_','-').split("[-.@]",-1)[0];
        boolean useEnglish=Arrays.asList("en","zh","ru","ja","ko","de","fr","es","pt").contains(base);
        String preferred=useEnglish?english:vietnamese, fallback=useEnglish?vietnamese:english;
        return !preferred.isEmpty()?preferred:!fallback.isEmpty()?fallback:value.trim();
    }
    private static String section(String text,String heading){
        Matcher match=Pattern.compile("(?m)^## "+Pattern.quote(heading)+"[ \\t]*$").matcher(text);
        if(!match.find())return "";
        int end=text.indexOf("\n## ",match.end());
        return text.substring(match.end(),end<0?text.length():end).trim();
    }
    private static String load(Context context){
        try{return LicenseCatalog.read(context,"PRIVACY.md");}catch(IOException error){return null;}
    }
    ScrollView view(MobileUi ui){
        ScrollView scroll=new ScrollView(ui.context);scroll.setBackgroundColor(android.graphics.Color.WHITE);
        LinearLayout content=ui.column();content.setPadding(ui.dp(20),ui.dp(20),ui.dp(20),ui.dp(20));scroll.addView(content);
        for(String paragraph:text.split("\n\n")){
            paragraph=paragraph.trim();if(paragraph.isEmpty()||paragraph.equals("---"))continue;
            boolean heading=paragraph.startsWith("#"),date=paragraph.startsWith("更新日期：")||paragraph.startsWith("Last updated:");
            String value=heading?paragraph.replaceFirst("^#+\\s*",""):paragraph.replace("**","");
            TextView label=ui.text(value,date?12:16,heading);label.setTextIsSelectable(true);label.setLineSpacing(ui.dp(4),1);
            if(date)label.setTextColor(SECONDARY);
            LinearLayout.LayoutParams spacing=new LinearLayout.LayoutParams(-1,-2);spacing.bottomMargin=ui.dp(14);content.addView(label,spacing);
        }
        return scroll;
    }
}
