# monitoring-test
EV3のモニターに表示するためのテスト用です。
もちろん走りません笑

起動時に尻尾を完全停止位置まで降ろし、
タッチセンサーで環境光、反射光、色などを測定し、モニターに表示します。
３回まで測定でき、ランプが緑→オレンジ→赤と点灯し、消灯すると終了です。
モニターへの表示は最終行までくると一旦クリアして先頭の行から書き出します。

モニターに表示した内容はSDカードのログに書き込みます。
ファイルへの書き込みは、monitaoring-test完了時なので途中で強制終了すると保存されません。
それまではバッファにしか溜めてません。
しかし、512byteを超える長さがバッファに書き込まれると、SDカードに書き出してバッファをクリアします。
