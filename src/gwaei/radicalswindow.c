/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file radicalswindow.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>
#include <gwaei/radicalswindow.h>
#include <gwaei/radicalswindow-private.h>

static void gw_radicalswindow_fill_radicals (GwRadicalsWindow*);
static void gw_radicalswindow_init_accelerators (GwRadicalsWindow*);

static char *_radical_array[][5] =
{
  //{Strokes, Representative_radical, Actual_radical, Name, NULL}}
  {"1", "一", "一", "いち", NULL },
  {"1", "｜", "｜", "ぼう、たてぼう", NULL },
  {"1", "丶", "丶", "てん", NULL },
  {"1", "ノ", "ノ", "の、はらいぼう", NULL },
  {"1", "乙", "乙", "オツ、おつにょう、つりばり", NULL },
  {"1", "亅", "亅", "はねぼう、ケツ、かぎ", NULL },

  {"2", "二", "二", "二", NULL },
  {"2", "亠", "亠", "けいさんかんむり、なべぶた、けさんかんむり", NULL },
  {"2", "人", "亻", "ひと、にんべん、ひとがしら、ひとやね", NULL },
  {"2", "化", "化", "にんにょう、ひとあし", NULL },
  {"2", "个", "个", "", NULL },
  {"2", "儿", "儿", "にんにょう、ひとあし", NULL },
  {"2", "入", "入", "いる、いりがしら、いりやね、ニュウ", NULL },
  {"2", "ハ", "ハ", "いる、いりがしら、いりやね、ニュウ", NULL },
  {"2", "冂", "冂", "けいがまえ、まきがまえ、どうがまえ、えんがまえ", NULL },
  {"2", "冖", "冖", "わかんむり べきかんむり", NULL },
  {"2", "冫", "冫", "にすい", NULL },
  {"2", "几", "几", "つくえ、きにょう、つくえきにょう、かぜかんむり、かぜがまえ", NULL },
  {"2", "凵", "凵", "うけばこ かんにょう", NULL },
  {"2", "刀", "刀", "かたな", NULL },
  {"2", "刈", "刂", "りっとう", NULL },
  {"2", "力", "力", "ちから", NULL },
  {"2", "勹", "勹", "つつみがまえ", NULL },
  {"2", "匕", "匕", "ヒ、さじ、さじのひ", NULL },
  {"2", "匚", "匚", "はこがまえ", NULL },
  {"2", "十", "十", "ジュウ", NULL },
  {"2", "卜", "卜", "ボク、ぼくのと、うらない", NULL },
  {"2", "卩", "卩", "ふしづくり、まげわりふ、わりふ", NULL },
  {"2", "厂", "厂", "がんだれ", NULL },
  {"2", "厶", "厶", "む", NULL },
  {"2", "又", "又", "また", NULL },
  {"2", "込", "辶", "しんにょう、しんにゅう", NULL }, //ODD
  {"2", "九", "九", "く", NULL }, //ODD
  {"2", "マ", "マ", "ま", NULL }, //ODD
  {"2", "ユ", "ユ", "ゆ", NULL }, //ODD
  {"2", "乃", "乃", "", NULL }, //ODD

  {"3", "口", "口", "くち、くちへん", NULL },
  {"3", "囗", "囗", "くにがまえ", NULL },
  {"3", "土", "土", "つち、つちへん", NULL },
  {"3", "士", "士", "さむらい、さむらいかんむり", NULL },
  {"3", "夂", "夂", "ふゆがしら、ちかんむり、のまたかんむり", NULL },
  //夊 - すいにょう、なつあし //MISSING
  {"3", "夕", "夕", "ゆう、ゆうべ、タ", NULL },
  {"3", "大", "大", "ダイ、だいかんむり、だいかしら", NULL },
  {"3", "女", "女", "おんな、おんなへん", NULL },
  {"3", "子", "子", "こ、こへん、こども、こどもへん", NULL },
  {"3", "宀", "宀", "うかんむり", NULL },
  {"3", "寸", "寸", "スン", NULL },
  {"3", "小", "小", "ショウ、ちいさい、しょうがしら、なおがしら", NULL },
  {"3", "尚", "尚", "", NULL }, //ODD
  {"3", "尢", "尢", "尣 - だいのまげあし、まげあし、おうにょう、オウ", NULL },
  {"3", "尸", "尸", "しかばね、しかばねかんむり、かばね、かばねだれ", NULL },
  {"3", "屮", "屮", "テツ、くさのめ、めばえ", NULL },
  {"3", "山", "山", "やま、やまへん", NULL },
  {"3", "川", "川", "", NULL }, //ODD
  {"3", "巛", "巛", "まがりかわ、かわ、さんぽがわ", NULL },
  {"3", "工", "工", "コウ、たくみへん、たくみ", NULL },
  {"3", "已", "已", "コ、キ、おのれ、イ、すでに、シ、み", NULL },
  {"3", "巾", "巾", "はば、はばへん、きんへん、きんべん", NULL },
  {"3", "干", "干", "カン、いちじゅう、ほす", NULL },
  {"3", "幺", "幺", "ヨウ、いとがしら", NULL },
  {"3", "广", "广", "まだれ", NULL },
  {"3", "廴", "廴", "えんにょう、えんにゅう、いんにょう", NULL },
  {"3", "廾", "廾", "キョウ、こまぬき、にじゅうあし", NULL },
  {"3", "弋", "弋", "ヨク、しきがまえ", NULL },
  {"3", "弓", "弓", "ゆみ、ゆみへん", NULL },
  {"3", "彑", "彑", "", NULL },  //ODD
  {"3", "ヨ", "彐", "けいがしら、いのこがしら", NULL },
  {"3", "彡", "彡", "さんづくり、けかざり、かみかざり", NULL },
  {"3", "彳", "彳", "ぎょうにんべん", NULL },
  {"3", "忙", "忄", "りっしんべん", NULL },
  {"3", "扎", "扌", "てへん", NULL }, //MISSING
  {"3", "汁", "氵", "さんずい", NULL },
  {"3", "犯", "犯", "", NULL },
  {"3", "艾", "艾", "", NULL },
  {"3", "込", "込", "", NULL },
  {"3", "邦", "邦", "", NULL },
  {"3", "阡", "阝", "おおざと", NULL },
  {"3", "并", "并", "", NULL },
  {"3", "也", "也", "", NULL },
  {"3", "亡", "亡", "", NULL },
  {"3", "及", "及", "", NULL },
  {"3", "久", "久", "", NULL },

  {"4", "心", "心", "こころ、したごころ", NULL },
  {"4", "戈", "戈", "ほこ、ほこづくり、ほこがまえ、たすき、かのほこ", NULL },
  {"4", "戸", "戸", "と、とかんむり、とだれ、とびらのと", NULL },
  {"4", "手", "手", "て", NULL },
  {"4", "支", "支", "しにょう、えだにょう、じゅうまた", NULL },
  {"4", "攵", "攵", "", NULL }, //MISSING
  {"4", "文", "文", "ブン、ぶんにょう、ふみつくり", NULL },
  {"4", "斗", "斗", "とます、ます、ト", NULL },
  {"4", "斤", "斤", "おの、おのづくり、キン", NULL },
  {"4", "方", "方", "ホウ、ほうへん、かたへん", NULL },
  {"4", "无", "无", "なし、ブ、むにょう、すでのつくり", NULL },
  {"4", "日", "日", "ひ、にち、ひへん、にちへん", NULL },
  {"4", "曰", "曰", "ひらび、いわく", NULL },
  {"4", "月", "月", "つき、つきへん、ふなづき、にくつき", NULL },
  {"4", "木", "木", "き、きへん", NULL },
  {"4", "欠", "欠", "あくび、かける", NULL },
  {"4", "止", "止", "とめる、とめへん", NULL },
  {"4", "歹", "歹", "ガツ、がつへん、かばねへん、しにがまえ、いちたへん", NULL },
  {"4", "殳", "殳", "ほこづくり、ほこ、るまた", NULL },
  {"4", "母", "母", "なかれ、はは、ははのかん", NULL },
  {"4", "毋", "毋", "なかれ、はは、ははのかん", NULL },
  {"4", "比", "比", "ならびひ、くらべる", NULL },
  {"4", "毛", "毛", "け", NULL },
  {"4", "氏", "氏", "うじ", NULL },
  {"4", "气", "气", "きがまえ", NULL },
  {"4", "水", "水", "みず、したみず", NULL },
  {"4", "火", "火", "ひ、ひへん", NULL },
  {"4", "杰", "灬", "れっか、れんが", NULL },
  {"4", "爪", "爪", "つめ", NULL },
  {"4", "父", "父", "ちち", NULL },
  {"4", "爻", "爻", "コウ", NULL },
  {"4", "爿", "爿", "ショウ、しょうへん", NULL },
  {"4", "片", "片", "かた、かたへん", NULL },
  {"4", "牙", "牙", "きば、きばへん", NULL },
  {"4", "牛", "牛", "うし", NULL },
  {"4", "犬", "犬", "いぬ", NULL },
  {"4", "王", "王", "おうへん", NULL },
  {"4", "礼", "礻", "しめすへん、ねへん", NULL }, //MISSING
  {"4", "老", "老", "", NULL }, //MISSING
  {"4", "肋", "肋", "", NULL }, //MISSING
  {"4", "勿", "勿", "", NULL }, //MISSING
  {"4", "井", "井", "", NULL }, //MISSING
  {"4", "尤", "尤", "", NULL }, //MISSING
  {"4", "五", "五", "", NULL }, //MISSING
  {"4", "巴", "巴", "", NULL }, //MISSING
  {"4", "屯", "屯", "", NULL }, //MISSING
  {"4", "元", "元", "", NULL }, //MISSING

  {"5", "玄", "玄", "ゲン", NULL },
  {"5", "玉", "玉", "たま、たまへん", NULL },
  {"5", "瓜", "瓜", "うり", NULL },
  {"5", "瓦", "瓦", "かわら", NULL },
  {"5", "甘", "甘", "あまい、カン", NULL },
  {"5", "生", "生", "セイ、ショウ、いきる、うまれる", NULL },
  {"5", "用", "用", "ヨウ、もちいる", NULL },
  {"5", "田", "田", "た、たへん", NULL },
  {"5", "疋", "疋", "ヒキ、ひきへん", NULL },
  {"5", "疔", "疒", "やまいだれ", NULL },
  {"5", "癶", "癶", "はつがしら", NULL },
  {"5", "白", "白", "しろ、しろへん", NULL },
  {"5", "皮", "皮", "けがわ、ひのかわ", NULL },
  {"5", "皿", "皿", "さら", NULL },
  {"5", "目", "目", "め、めへん、よこめ", NULL },
  {"5", "矛", "矛", "ほこ、ほこへん", NULL },
  {"5", "矢", "矢", "や、やへん", NULL },
  {"5", "石", "石", "いし、いしへん", NULL },
  {"5", "示", "示", "しめす", NULL },
  {"5", "禹", "禹", "", NULL }, //MISSING
  {"5", "禾", "禾", "いね、いねへん、のぎ、のぎへん", NULL },
  {"5", "穴", "穴", "あな、あなかんむり", NULL },
  {"5", "立", "立", "たつ、たつへん", NULL },
  {"5", "買", "罒", "よこめ", NULL },
  {"5", "初", "衤", "ころもへん", NULL }, //MISSING
  {"5", "巨", "巨", "", NULL }, //MISSING
  {"5", "世", "世", "", NULL }, //MISSING
  {"5", "冊", "冊", "", NULL }, //MISSING

  {"6", "竹", "竹", "たけ、たけかんむり", NULL },
  {"6", "米", "米", "こめ、こめへん", NULL },
  {"6", "糸", "糸", "いと、いとへん", NULL },
  {"6", "缶", "缶", "ほとぎ、ほとぎへん、フ、カン", NULL },
  {"6", "羊", "羊", "ひつじ、ひつじへん", NULL },
  {"6", "羽", "羽", "はね", NULL },
  {"6", "老", "耂", "おいる、おいかんむり、おいがしら", NULL },
  {"6", "而", "而", "しこうして、しかして", NULL },
  {"6", "耒", "耒", "いすき、すきへん", NULL },
  {"6", "耳", "耳", "みみ、みみへん", NULL },
  {"6", "聿", "聿", "イツ、ふでづくり", NULL },
  {"6", "肉", "肉", "ニク、にくづき", NULL },
  {"6", "自", "自", "ジ、みずから", NULL },
  {"6", "至", "至", "いたる、いたるへん", NULL },
  {"6", "臼", "臼", "うす", NULL },
  {"6", "舌", "舌", "した、したへん", NULL },
  {"6", "舛", "舛", "まいあし、ます", NULL },
  {"6", "舟", "舟", "ふね、ふねへん", NULL },
  {"6", "艮", "艮", "コン、ゴン、こんづくり、ごんづくり、ねづくり、うしとら", NULL },
  {"6", "色", "色", "いろ", NULL },
  {"6", "虍", "虍", "とらかんむり、とらがしら", NULL },
  {"6", "虫", "虫", "むし、むしへん", NULL },
  {"6", "血", "血", "ち", NULL },
  {"6", "行", "行", "ゆきがまえ、ぎょうがまえ", NULL },
  {"6", "衣", "衣", "ころも", NULL },
  {"6", "西", "西", "にし", NULL },
//  {"6", "西", "覀", "おおいかんむり", NULL },

  {"7", "臣", "臣", "", NULL },
  {"7", "見", "見", "みる", NULL },
  {"7", "角", "角", "つの、つのへ", NULL },
  {"7", "言", "言", "ことば、ゲン、ごんべん", NULL },
  {"7", "谷", "谷", "たに、たにへん", NULL },
  {"7", "豆", "豆", "まめ、まめへん", NULL },
  {"7", "豕", "豕", "いのこ、いのこへん、ぶた", NULL },
  {"7", "豸", "豸", "むじなへん", NULL },
  {"7", "貝", "貝", "かい、かいへん、こがい", NULL },
  {"7", "赤", "赤", "あか、あかへん", NULL },
  {"7", "走", "走", "はしる、そうにょう", NULL },
  {"7", "足", "足", "あし、あしへん", NULL },
  {"7", "身", "身", "み、みへん", NULL },
  {"7", "車", "車", "くるま、くるまへん", NULL },
  {"7", "辛", "辛", "シン、からい", NULL },
  {"7", "辰", "辰", "しんのたつ", NULL },
  {"7", "酉", "酉", "とりへん、ひよみのとり、さけのとり、とり", NULL },
  {"7", "釆", "釆", "のごめ、のごめへん", NULL },
  {"7", "里", "里", "さと、さとへん", NULL },
  {"7", "麦", "麦", "むぎ、ばくにょう", NULL },

  {"8", "金", "金", "かね、かねへん", NULL },
  {"8", "長", "長", "ながい", NULL },
  {"8", "門", "門", "モン、もんがまえ、かどがまえ", NULL },
  {"8", "隶", "隶", "れいづくり", NULL },
  {"8", "隹", "隹", "", NULL }, //MISSING
  {"8", "雨", "雨", "あめ、あめかんむり", NULL },
  {"8", "青", "青", "あお", NULL },
  {"8", "非", "非", "あらず、ヒ", NULL },
  {"8", "斉", "斉", "セイ", NULL }, //MISSING
  {"8", "岡", "岡", "", NULL }, //MISSING
  {"8", "奄", "奄", "", NULL }, //MISSING
  {"8", "免", "免", "", NULL }, //MISSING

  {"9", "面", "面", "メン", NULL },
  {"9", "革", "革", "かわへん、つくりがわ", NULL },
  {"9", "韋", "韋", "なめしがわ", NULL },
  {"9", "音", "音", "", NULL }, //MISSING
  {"9", "頁", "頁", "おおがい", NULL },
  {"9", "風", "風", "かぜ", NULL },
  {"9", "飛", "飛", "とぶ", NULL },
  {"9", "食", "食", "ショク、しょくへん", NULL },
  {"9", "首", "首", "くび", NULL },
  {"9", "香", "香", "かおり、カ", NULL },
  {"9", "品", "品", "", NULL }, //MISSING

  {"10", "馬", "馬", "うま、うまへん", NULL },
  {"10", "骨", "骨", "ほね、ほねへん", NULL },
  {"10", "高", "高", "たかい", NULL },
  {"10", "髟", "髟", "かみかんむり、かみがしら", NULL },
  {"10", "鬥", "鬥", "とうがまえ、たたかいがまえ", NULL },
  {"10", "鬯", "鬯", "チョウ、においざけ", NULL },
  {"10", "鬲", "鬲", "かなえ、レキ", NULL },
  {"10", "鬼", "鬼", "おに、きにょう", NULL },
  {"10", "竜", "竜", "リュウ", NULL },

  {"11", "魚", "魚", "うお、さかな、うおへん", NULL },
  {"11", "鳥", "鳥", "とり、とりへん", NULL },
  {"11", "鹵", "鹵", "しお、ロ", NULL },
  {"11", "鹿", "鹿", "しか", NULL },
  {"11", "麻", "麻", "あさ、あさかんむり", NULL },
  {"11", "亀", "亀", "かめ", NULL },

  {"12", "黄", "黄", "き", NULL },
  {"12", "黍", "黍", "きび", NULL },
  {"12", "黒", "黒", "きび", NULL },
  {"12", "黹", "黹", "ぬいとり、ふつへん、チ", NULL },
  {"12", "無", "無", "", NULL }, //MISSING

  {"13", "黽", "黽", "べんあし、かえる、ベン", NULL },
  {"13", "鼓", "鼓", "つづみ", NULL },
  {"13", "鼠", "鼠", "ねずみ、ねずみへん", NULL },

  {"14", "鼻", "鼻", "はな、はなへん", NULL },
  {"14", "齊", "齊", "(斉) セイ", NULL },

  {"17", "龠", "龠", "ヤク、ふえ", NULL },

  {NULL}
};

G_DEFINE_TYPE (GwRadicalsWindow, gw_radicalswindow, GW_TYPE_WINDOW)


GtkWindow* 
gw_radicalswindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwRadicalsWindow *window;

    //Initializations
    window = GW_RADICALSWINDOW (g_object_new (GW_TYPE_RADICALSWINDOW,
                                              "type",        GTK_WINDOW_TOPLEVEL,
                                              "application", GW_APPLICATION (application),
                                              "ui-xml",      "radicalswindow.ui",
                                              NULL));

    return GTK_WINDOW (window);
}


static void 
gw_radicalswindow_init (GwRadicalsWindow *window)
{
    window->priv = GW_RADICALSWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwRadicalsWindowPrivate));
}


static void 
gw_radicalswindow_finalize (GObject *object)
{
/*
    GwRadicalsWindow *window;

    window = GW_RADICALSWINDOW (object);
*/
    G_OBJECT_CLASS (gw_radicalswindow_parent_class)->finalize (object);
}


static void 
gw_radicalswindow_constructed (GObject *object)
{
    //Declarations
    GwRadicalsWindow *window;
    GwRadicalsWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkWidget *scrolledwindow;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_radicalswindow_parent_class)->constructed (object);
    }

    window = GW_RADICALSWINDOW (object);
    priv = window->priv;

    toplevel = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "radical_selection_grid"));
    gtk_widget_set_halign (toplevel, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (toplevel, GTK_ALIGN_START);

    scrolledwindow = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "radical_selection_scrolledwindow"));
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_NEVER);

    g_signal_connect (G_OBJECT (window), "show", G_CALLBACK (gw_radicalswindow_show_cb), scrolledwindow);

    gtk_window_set_title (GTK_WINDOW (window), gettext("gWaei Radical Search Tool"));
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");

    priv->radicals_grid = GTK_GRID (gw_window_get_object (GW_WINDOW (window), "radical_selection_grid"));
    priv->strokes_checkbutton = GTK_TOGGLE_BUTTON (gw_window_get_object (GW_WINDOW (window), "strokes_checkbox"));
    priv->strokes_spinbutton = GTK_SPIN_BUTTON (gw_window_get_object (GW_WINDOW (window), "strokes_spinbutton"));

    gtk_spin_button_set_value (priv->strokes_spinbutton, 1.0);
    gw_radicalswindow_fill_radicals (window);
    gtk_widget_show_all (GTK_WIDGET (priv->radicals_grid));

    gw_radicalswindow_init_accelerators (window);

    gw_window_unload_xml (GW_WINDOW (window));
}


static void
gw_radicalswindow_class_init (GwRadicalsWindowClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = gw_radicalswindow_constructed;
    object_class->finalize = gw_radicalswindow_finalize;

    g_type_class_add_private (object_class, sizeof (GwRadicalsWindowPrivate));

    klass->signalid[GW_RADICALSWINDOW_CLASS_SIGNALID_QUERY_CHANGED] = g_signal_new (
        "query-changed",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GwRadicalsWindowClass, query_changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 
        0
    );
}


static void
gw_radicalswindow_init_accelerators (GwRadicalsWindow *window)
{
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "close_button"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);
}


static void 
gw_radicalswindow_fill_radicals (GwRadicalsWindow *window)
{
    //Declarations
    GwRadicalsWindowPrivate *priv;
    gint i, column, row;
    const gchar *stroke;
    GtkWidget *button;
    GtkWidget *label;
    gchar *tooltip;
    gchar *markup;
    const gint total_columns = 14;

    //Initializations
    priv = window->priv;

    stroke = NULL;
    button = NULL;
    label = NULL;
    i = row = column = 0;

    while (_radical_array[i][0] != NULL)
    {
      //Add a new stroke label
      if (stroke == NULL || strcmp(stroke, _radical_array[i][GW_RADARRAY_STROKES]) != 0)
      {
        stroke = _radical_array[i][GW_RADARRAY_STROKES];
        label = gtk_label_new (stroke);
        markup = g_markup_printf_escaped ("<span color=\"red\"><b>%s</b></span>", _radical_array[i][GW_RADARRAY_STROKES]);
        if (markup != NULL)
        {
          gtk_label_set_markup (GTK_LABEL (label), markup);
          g_free (markup);
        }
        gtk_grid_attach (priv->radicals_grid, label, column, row, 1, 1);
      }
      //Add a radical button
      else
      {
        button = gtk_toggle_button_new_with_label (_radical_array[i][GW_RADARRAY_ACTUAL]);
        gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_HALF);

        tooltip = g_markup_printf_escaped (
            gettext("<b>Substitution Radical:</b> %s\n<b>Actual Radical:</b> %s\n<b>Radical Name:</b> %s"),
            _radical_array[i][GW_RADARRAY_REPRESENTATIVE], 
            _radical_array[i][GW_RADARRAY_ACTUAL], 
            _radical_array[i][GW_RADARRAY_NAME]
        );
        if (tooltip != NULL)
        {
          gtk_widget_set_tooltip_markup (GTK_WIDGET (button), tooltip);
          g_free (tooltip); tooltip = NULL;
        }
        gtk_buildable_set_name (GTK_BUILDABLE (button), _radical_array[i][GW_RADARRAY_REPRESENTATIVE]);
        g_signal_connect(G_OBJECT (button), "toggled", G_CALLBACK (gw_radicalswindow_toggled_cb), window);
        gtk_grid_attach (priv->radicals_grid, button, column, row, 1, 1);
        i++;
      }

      column++;
      if (column == total_columns)
      {
        row++;
        column = 0;
      }
    }
}


//!
//! @brief Copies all the lables of the depressed buttons in the radicals window
//!
//!
char* 
gw_radicalswindow_strdup_all_selected (GwRadicalsWindow *window)
{
    //Declarations
    GwRadicalsWindowPrivate *priv;
    GList *list;
    GList *iter;
    char *temp_string;
    char *final_string;
    const char *label_text;
    gboolean a_button_was_in_pressed_state;

    //Initializations
    priv = window->priv;
    list = gtk_container_get_children (GTK_CONTAINER (priv->radicals_grid));
    temp_string = NULL;
    final_string = NULL;
    label_text = NULL;
    priv->cache[0] = '\0';
    a_button_was_in_pressed_state = FALSE;

    //Probe all of the active toggle buttons in the table
    for (iter = list; iter != NULL; iter = iter->next)
    {
      if (G_OBJECT_TYPE(iter->data) == g_type_from_name("GtkToggleButton"))
      {
         label_text = gtk_buildable_get_name (GTK_BUILDABLE (iter->data));
         if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (iter->data)))
         {
           a_button_was_in_pressed_state = TRUE;
           if (final_string == NULL)
           {
             final_string = g_strdup_printf ("%s", label_text);
           }
           else
           {
             temp_string = g_strdup_printf ("%s%s", final_string, label_text);
             g_free (final_string);
             final_string = temp_string;
             temp_string = NULL;
           }
         }
         else
         {
           gtk_widget_set_sensitive (GTK_WIDGET (iter->data), FALSE);
         }
         strcat (priv->cache, label_text);
      }
    }

    if (!a_button_was_in_pressed_state)
      gw_radicalswindow_deselect_all_radicals (window);

    if (final_string == NULL)
      final_string = g_strdup ("");

    g_list_free (list);

    return final_string;
}


//!
//! @brief Finds the radical button with the string label and sets it sensitive
//!
//! @param string The label to search for
//!
void 
gw_radicalswindow_set_button_sensitive_when_label_is (GwRadicalsWindow *window, const char *string)
{
    //Sanity check
    if (string == NULL) return;

    //Declarations
    GwRadicalsWindowPrivate *priv;
    GList *list;
    GList *iter;
    const char *label_text;
    const char *jump;
    char radical[4];
    GType type;

    //Initializations
    priv = window->priv;
    label_text = NULL;
    jump = string;
    list = gtk_container_get_children (GTK_CONTAINER (priv->radicals_grid));
    type = g_type_from_name ("GtkToggleButton");

    if (jump[0] != '\0' && jump[1] != '\0' && jump[2] != '\0')
    {
      radical[0] = jump[0];
      radical[1] = jump[1];
      radical[2] = jump[2];
      radical[3] = '\0';

      for (iter = list; iter != NULL; iter = iter->next)
      {
        if (G_OBJECT_TYPE (iter->data) == type)
        {
           label_text = gtk_buildable_get_name (GTK_BUILDABLE (iter->data));
           if (strcmp(label_text, radical) == 0)
             gtk_widget_set_sensitive (GTK_WIDGET (iter->data), TRUE);
        }
      }
    }
    while ((jump = g_utf8_strchr (jump, -1, L' ')))
    {
      jump++;
      if (jump[0] != '\0' && jump[1] != '\0' && jump[2] != '\0')
      {
        radical[0] = jump[0];
        radical[1] = jump[1];
        radical[2] = jump[2];
        radical[3] = '\0';

        for (iter = list; iter != NULL; iter = iter->next)
        {
          if (G_OBJECT_TYPE (iter->data) == type)
          {
             label_text = gtk_buildable_get_name (GTK_BUILDABLE (iter->data));
             if (strcmp(label_text, radical) == 0)
              gtk_widget_set_sensitive (GTK_WIDGET (iter->data), TRUE);
          }
        }
      }
    }
    g_list_free(list);
}

//!
//! @brief Copies the stroke count in the prefered format
//!
//! @param output The string to copy the prefered stroke count to
//! @param MAX The max characters to copy
char* 
gw_radicalswindow_strdup_prefered_stroke_count (GwRadicalsWindow *window)
{
    //Declarations
    GwRadicalsWindowPrivate *priv;
    char *strokes;

    //If the checkbox is checked, get the stroke count from the spinner
    priv = window->priv;
    if (gtk_toggle_button_get_active(priv->strokes_checkbutton))
      strokes = g_strdup_printf ("s%d", (int) gtk_spin_button_get_value (priv->strokes_spinbutton));
    else
      strokes = g_strdup ("");

    return strokes;
}


//!
//! @brief Resets the states of all the radical buttons
//!
void 
gw_radicalswindow_deselect_all_radicals (GwRadicalsWindow *window)
{
    //Declarations
    GwRadicalsWindowPrivate *priv;
    GList* list, *iter;
    GType type;

    //Initializations
    priv = window->priv;
    list = gtk_container_get_children (GTK_CONTAINER (priv->radicals_grid));
    type = g_type_from_name ("GtkToggleButton");

    //Reset all of the toggle buttons
    for (iter = list; iter != NULL; iter = iter->next)
    {
      G_GNUC_EXTENSION g_signal_handlers_block_by_func (iter->data, gw_radicalswindow_toggled_cb, window);
      if (G_OBJECT_TYPE (iter->data) == type)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(iter->data), FALSE);
      G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (iter->data, gw_radicalswindow_toggled_cb, window);
      gtk_widget_set_sensitive (GTK_WIDGET (iter->data), TRUE);
    }

    g_list_free (list);
}



