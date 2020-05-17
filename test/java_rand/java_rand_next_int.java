import java.util.Random;

class java_rand_next_int  {
    public static void main(String[] args) {
        Random rand = new Random();
        rand.setSeed(25);
        for(int i = 0; i < 1000000; i++)
        {
            System.out.println(rand.nextInt());
        }
    }
}
