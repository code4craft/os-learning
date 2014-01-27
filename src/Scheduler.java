/**
 * @author yihua.huang@dianping.com
 */
public class Scheduler {

    public static void main(String[] args) {
        while (true){
            processing = next_Processing();
            processing.run();
            wait(100); //ms
            processing.interrupt();
        }
    }
}
