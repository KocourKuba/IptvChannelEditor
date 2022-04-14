<?php

/**
 * Class V32
 * @see https://github.com/Cyan4973/xxHash/wiki/xxHash-specification-(draft)
 */
class V32
{
    const PRIME_1 = 2654435761;
    const PRIME_2 = 2246822519;
    const PRIME_3 = 3266489917;
    const PRIME_4 =  668265263;
    const PRIME_5 =  374761393;

    /**
     * @var int
     */
    private $seed;

    /**
     * @param int $seed
     */
    public function __construct($seed = 0)
    {
        $this->seed = $seed;
    }

    /**
     * @param string $input
     * @return string
     */
    public function hash($input)
    {
        $file = tmpfile();
        fwrite($file, $input);
        rewind($file);

        if (!isset($this)) {
            $hash = new self();
            return $hash->hashStream($file);
        }

        return $this->hashStream($file);
    }

    /**
     * @param resource $input
     * @return string
     */
    public function hashStream($input)
    {
        if (!isset($this)) {
            $hash = new self();
            return $hash->hashStream($input);
        }

        if (get_resource_type($input) !== 'stream') {
            throw new InvalidArgumentException();
        }

        $accumulators = array(
            $this->add($this->add($this->seed, self::PRIME_1),self::PRIME_2),
            $this->add($this->seed, self::PRIME_2), $this->seed, $this->minus($this->seed, self::PRIME_1)
        );


        $accumulator = $this->add($this->seed, self::PRIME_5);
        $length = 0;
        while (($part = fread($input, 16)) && ($length += strlen($part)) && strlen($part) === 16) {
            for ($iteration = 0; $iteration < 4; $iteration++) {
                $lane = $this->getLane($part, ($iteration * 4));

                $current = &$accumulators[$iteration];
                $current = $this->add(
                    $current,
                    $this->multiply($lane, self::PRIME_2)
                );
                $current = $this->leftShift($current, 13);
                $current = $this->multiply($current, self::PRIME_1);
            }
        }

        if ($length >= 16) {
            $accumulator = $this->accumulate($accumulators);
        }
        $accumulator = $this->add($accumulator, $length);
        return $this->smallInput($part, $accumulator);
    }

    /**
     * @param $input
     * @param $accumulator
     * @return string
     */
    private function smallInput($input, $accumulator)
    {

        list($accumulator, $i, $length) = $this->handle4ByteGroups($input, $accumulator);

        $accumulator = $this->handleLeftOverBytes($input, $accumulator, $length, $i);

        $accumulator = $this->finalise($accumulator);

        return dechex($accumulator);
    }

    /*
     * Below are the maths functions described in the documentation
     * These try to emulate a real 32 bit unsigned int
     */


    /**
     * 32 bit safe multiplication
     * @param $a
     * @param $b
     * @return int|string|null
     */
    public function multiply($a, $b)
    {
        $result = $a * $b;
        if (is_int($result) && $result > 0) {
            return $result % (1<<32);
        }

        //fall back to bc if the result overflows
        $result = bcmul((string) $a, (string) $b);
        return bcmod($result, 1<<32);
    }

    /**
     * Add but wrap at 32 bits
     * @param $a
     * @param $b
     * @return int
     */
    public function add($a, $b)
    {
        return ($a + $b) % (1<<32);
    }

    /**
     * function to emulate an unsigned minus
     * @param $a
     * @param $b
     * @return int
     */
    public function minus($a, $b)
    {
        $result = $a - $b;
        if ($result > 0) {
            return $result;
        }

        return $result & ((1<<32) -1);
    }

    /**
     * left shift wrap at 32 bits
     * @param $x
     * @param $bits
     * @return int
     */
    public function leftShift($x, $bits)
    {
        //left shift is a circular shift so shifted off bits become lower bits
        $circle = $x >> (32-$bits);
        return (($x << $bits) | $circle) % (1<<32);
    }

    /**
     * This function is to get the little endian byte
     * @param string $input
     * @param int $i
     * @return int
     */
    private function getLane($input, $i)
    {
        return
            (ord($input[$i + 3]) << 24) +
            (ord($input[$i + 2]) << 16) +
            (ord($input[$i + 1]) << 8) +
            (ord($input[$i]))
        ;
    }

    /**
     * @param $accumulator
     * @return int
     */
    private function finalise($accumulator)
    {
        $accumulator ^= ($accumulator >> 15);
        $accumulator = $this->multiply($accumulator, self::PRIME_2);
        $accumulator ^= ($accumulator >> 13);
        $accumulator = $this->multiply($accumulator, self::PRIME_3);
        $accumulator ^= ($accumulator >> 16);
        return $accumulator;
    }

    /**
     * @param $input
     * @param $accumulator
     * @param $length
     * @param $i
     * @return int
     */
    private function handleLeftOverBytes($input, $accumulator, $length, $i)
    {
        while ($length >= 1) {
            $lane = ord($input[$i]);
            $accumulator = $this->add($accumulator, $this->multiply($lane, self::PRIME_5));
            $accumulator = $this->multiply($this->leftShift($accumulator, 11), self::PRIME_1);
            ++$i;
            --$length;
        }
        return $accumulator;
    }

    /**
     * @param $input
     * @param $accumulator
     * @return array
     */
    private function handle4ByteGroups($input, $accumulator)
    {
        $length = strlen($input);
        $i = 0;
        while ($length >= 4) {
            $lane = $this->getLane($input, $i);
            $accumulator = $this->add($accumulator, $this->multiply($lane, self::PRIME_3));
            $accumulator = $this->multiply($this->leftShift($accumulator, 17),self::PRIME_4);

            $i += 4;
            $length -= 4;
        }
        return array($accumulator, $i, $length);
    }

    /**
     * @param $accumulators
     * @return int
     */
    private function accumulate($accumulators)
    {
        return
            $this->add(
                $this->add(
                    $this->leftShift($accumulators[0], 1),
                    $this->leftShift($accumulators[1], 7)
                ),
                $this->add(
                    $this->leftShift($accumulators[2], 12),
                    $this->leftShift($accumulators[3], 18)
                )
            );
    }
}
